/**
 * @file main.c
 * @brief Bio-Char Control Board v0.1.0 — Integrated Control System
 *        ESP-IDF port of the Arduino Dryness_test_PCB firmware
 *
 * Target:  ESP32-H2-DEVKITM-1-N4
 * SDK:     ESP-IDF v5.x
 *
 * Pin Configuration (corrected from PCB schematic — see notes below):
 *   GPIO0  → MAX31855 CLK
 *   GPIO1  → MAX31855 CS
 *   GPIO2  → MAX31855 DO (MISO — read-only, no MOSI needed)
 *   GPIO3  → Pressure Sensor ADC  (ADC1_CH2, 0–100 PSI via voltage divider)
 *   GPIO4  → SSR 1 / Heater       (active HIGH — drives BJT base to complete SSR circuit)
 *   GPIO5  → SSR 2                (reserved, not currently active — see commented code)
 *   GPIO10 → Solenoid Valve 1     (MOSFET-driven)
 *   GPIO11 → Solenoid Valve 2     (reserved, not currently active — see commented code)
 *
 * PIN CONFIGURATION NOTES:
 *   The original PCB schematic (v0.3.0) assigned GPIO0–GPIO7 differently based on the
 *   KiCad footprint. After bring-up, GPIO8 and GPIO9 were found to be internally tied
 *   to the ESP32-H2's 32 MHz crystal oscillator and cannot be used as general-purpose I/O.
 *   GPIO14 and GPIO15 are similarly reserved for the 32 kHz RTC crystal on some variants.
 *   The SPI and ADC assignments above reflect the remapped working configuration.
 *
 * HEATER/SSR CHANGE vs ARDUINO VERSION:
 *   The Arduino version used active LOW (GPIO4 LOW = SSR ON, sinking to GND).
 *   This board uses a BJT (NPN) between the ESP32 GPIO and the SSR input:
 *     GPIO4 HIGH → BJT base HIGH → collector pulls SSR input LOW → SSR ON
 *     GPIO4 LOW  → BJT OFF → SSR input floating/HIGH → SSR OFF
 *   set_heater() has been updated accordingly.
 *
 * KNOWN ERRORS / ISSUES:
 *   1. ADC non-linearity at low raw values (<350): readings below ~0.5 PSI may
 *      fluctuate. The DRY_PRESSURE_MAX threshold (1.0 PSI) provides enough
 *      headroom for reliable detection.
 *   2. MAX31855 returns NAN on first read after power-on (~100ms settling time).
 *      The EMA initialisation handles this gracefully — temp_ema stays NAN until
 *      a valid reading arrives.
 *   3. GPIO3 on ESP32-H2 is shared with the JTAG interface (MTDI). Disconnect
 *      the JTAG debugger before running in production, or remap to GPIO4/5 if
 *      debugging under load is required.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "biochar";

// ===============================================================
// -------------------- MAX31855 THERMOCOUPLE --------------------
// ===============================================================
#define PIN_MAXCLK   GPIO_NUM_0    // MAX31855 Clock (SPI CLK)
#define PIN_MAXCS    GPIO_NUM_1    // MAX31855 Chip Select
#define PIN_MAXDO    GPIO_NUM_2    // MAX31855 Data Out (MISO — read-only)

// ===============================================================
// -------------------------- PINS --------------------------------
// ===============================================================
#define PIN_ADC_PRESSURE  ADC_CHANNEL_2   // GPIO3 — Pressure sensor (ADC1_CH2)
#define PIN_VALVE         GPIO_NUM_10     // GPIO10 — Solenoid Valve 1 (MOSFET)
#define PIN_HEATER        GPIO_NUM_4      // GPIO4 — SSR 1 (active HIGH → BJT → SSR)

// Reserved for future use — uncomment when second SSR and valve are needed
// #define PIN_SSR2    GPIO_NUM_5    // GPIO5  — SSR 2 circuit (not currently active)
// #define PIN_VALVE2  GPIO_NUM_11   // GPIO11 — Solenoid Valve 2 (not currently active)

// ===============================================================
// ------------------- PRESSURE CALIBRATION ----------------------
// ===============================================================
#define ADC_ZERO      320.0f    // ADC raw value at 0 PSI
#define ADC_FULL      3008.0f   // ADC raw value at 100 PSI

// ===============================================================
// --------------------- VALVE HYSTERESIS ------------------------
// ===============================================================
#define PSI_ON_THRESHOLD   8.0f   // Open valve above this pressure
#define PSI_OFF_THRESHOLD  7.0f   // Close valve below this pressure

// ===============================================================
// ------------------- DRY DETECTION (PRESSURE) ------------------
// ===============================================================
#define CYCLE_START_PSI   2.0f       // Pressure must exceed this to start a cycle
#define DRY_PRESSURE_MAX  1.0f       // PSI below which dry detection is considered
#define DRY_TIME_MS       15000UL    // Must stay dry for this long to latch

// ===============================================================
// ------------------ HEATER TEMPERATURE CONTROL -----------------
// ===============================================================
#define SETPOINT_C   200.0f    // Target temperature (°C)
#define HYST_C       2.0f      // Hysteresis band (°C)
#define KP           0.03f     // Proportional gain
#define EMA_ALPHA    0.10f     // Exponential moving average weight
#define WINDOW_MS    2000      // PWM window period (ms)

// ===============================================================
// --------------------------- STATE -----------------------------
// ===============================================================
static bool    valve_on            = false;
static bool    cycle_active        = false;
static bool    dry_latched         = false;
static int64_t dry_candidate_start = 0;
static float   temp_ema            = NAN;
static int64_t window_start        = 0;

static spi_device_handle_t       max31855_handle;
static adc_oneshot_unit_handle_t adc_handle;

// ===============================================================
// -------------------------- HELPERS ----------------------------
// ===============================================================
static float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static float adc_to_psi(int adc_raw) {
    return fmap((float)adc_raw, ADC_ZERO, ADC_FULL, 0.0f, 100.0f);
}

static void set_heater(bool on) {
    // Active HIGH: GPIO4 HIGH → BJT base HIGH → collector completes SSR circuit → SSR ON
    // Active LOW (off):  GPIO4 LOW  → BJT OFF → SSR OFF
    gpio_set_level(PIN_HEATER, on ? 1 : 0);
}

// ===============================================================
// -------------------- MAX31855 SPI READ ------------------------
// ===============================================================
/**
 * Reads a 32-bit frame from the MAX31855 over SPI.
 *
 * Frame layout:
 *   Bits [31:18] — Thermocouple temp, 14-bit signed, 0.25°C/LSB
 *   Bit  [16]   — Fault bit (1 = fault present)
 *   Bits [15:4] — Internal junction temp (not used here)
 *   Bits [3:0]  — Fault flags (OC, SCG, SCV)
 */
static esp_err_t max31855_read(float *temp_c, bool *fault) {
    uint8_t rx_buf[4] = {0};

    spi_transaction_t t = {
        .length    = 32,
        .rx_buffer = rx_buf,
    };

    esp_err_t ret = spi_device_polling_transmit(max31855_handle, &t);
    if (ret != ESP_OK) return ret;

    uint32_t raw = ((uint32_t)rx_buf[0] << 24) |
                   ((uint32_t)rx_buf[1] << 16) |
                   ((uint32_t)rx_buf[2] <<  8) |
                   ((uint32_t)rx_buf[3]);

    *fault = (raw & (1UL << 16)) != 0;

    // Sign-extend the 14-bit thermocouple value
    int16_t tc_raw = (int16_t)((raw >> 18) & 0x3FFF);
    if (tc_raw & 0x2000) tc_raw |= 0xC000;
    *temp_c = tc_raw * 0.25f;

    return ESP_OK;
}

// ===============================================================
// ----------------------- INITIALISATION ------------------------
// ===============================================================
static void init_spi(void) {
    spi_bus_config_t bus_cfg = {
        .miso_io_num   = PIN_MAXDO,
        .mosi_io_num   = -1,           // MAX31855 is read-only, no MOSI
        .sclk_io_num   = PIN_MAXCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1 * 1000 * 1000,  // 1 MHz (MAX31855 max: 5 MHz)
        .mode           = 0,                  // CPOL=0, CPHA=0
        .spics_io_num   = PIN_MAXCS,
        .queue_size     = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &max31855_handle));
}

static void init_adc(void) {
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten    = ADC_ATTEN_DB_12,   // Full 0–3.3 V range
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, PIN_ADC_PRESSURE, &chan_cfg));
}

static void init_gpio(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask  = (1ULL << PIN_VALVE) | (1ULL << PIN_HEATER),
        .mode          = GPIO_MODE_OUTPUT,
        .pull_up_en    = GPIO_PULLUP_DISABLE,
        .pull_down_en  = GPIO_PULLDOWN_ENABLE,   // Pull-down keeps BJT off at boot
        .intr_type     = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(PIN_VALVE,   0);   // Valve 1 OFF
    gpio_set_level(PIN_HEATER,  0);   // SSR 1 OFF (BJT base LOW)

    // Uncomment when SSR 2 and Valve 2 circuits are populated on the board:
    // gpio_config_t io_conf2 = {
    //     .pin_bit_mask  = (1ULL << PIN_SSR2) | (1ULL << PIN_VALVE2),
    //     .mode          = GPIO_MODE_OUTPUT,
    //     .pull_up_en    = GPIO_PULLUP_DISABLE,
    //     .pull_down_en  = GPIO_PULLDOWN_ENABLE,
    //     .intr_type     = GPIO_INTR_DISABLE,
    // };
    // ESP_ERROR_CHECK(gpio_config(&io_conf2));
    // gpio_set_level(PIN_SSR2,   0);
    // gpio_set_level(PIN_VALVE2, 0);
}

// ===============================================================
// ------------------------- MAIN TASK ---------------------------
// ===============================================================
static void control_task(void *arg) {
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║   SYSTEM READY: INTEGRATED CONTROL SYSTEM     ║");
    ESP_LOGI(TAG, "║   - Pressure Control + Valve 1                 ║");
    ESP_LOGI(TAG, "║   - Temperature Control + SSR 1 (Heater)       ║");
    ESP_LOGI(TAG, "║   - Dry Boiler Protection                      ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "Pin map:");
    ESP_LOGI(TAG, "  GPIO0  → MAX31855 CLK");
    ESP_LOGI(TAG, "  GPIO1  → MAX31855 CS");
    ESP_LOGI(TAG, "  GPIO2  → MAX31855 DO");
    ESP_LOGI(TAG, "  GPIO3  → Pressure Sensor ADC");
    ESP_LOGI(TAG, "  GPIO4  → SSR 1 (active HIGH, BJT driver)");
    ESP_LOGI(TAG, "  GPIO5  → SSR 2 (reserved)");
    ESP_LOGI(TAG, "  GPIO10 → Solenoid Valve 1");
    ESP_LOGI(TAG, "  GPIO11 → Solenoid Valve 2 (reserved)");

    window_start = esp_timer_get_time() / 1000LL;

    while (1) {
        int64_t now = esp_timer_get_time() / 1000LL;   // ms

        // ------------------- TEMPERATURE -------------------
        float temp_c = NAN;
        bool  fault  = false;
        esp_err_t tc_err   = max31855_read(&temp_c, &fault);
        bool  temp_valid   = (tc_err == ESP_OK) && !fault && !isnan(temp_c);

        if (temp_valid) {
            if (isnan(temp_ema)) temp_ema = temp_c;
            else temp_ema = EMA_ALPHA * temp_c + (1.0f - EMA_ALPHA) * temp_ema;
        }

        // -------------------- PRESSURE ---------------------
        int   adc_raw = 0;
        adc_oneshot_read(adc_handle, PIN_ADC_PRESSURE, &adc_raw);
        float psi = adc_to_psi(adc_raw);

        // ------------------- VALVE CONTROL -----------------
        if (!valve_on && psi >= PSI_ON_THRESHOLD)        valve_on = true;
        else if (valve_on && psi <= PSI_OFF_THRESHOLD)   valve_on = false;
        gpio_set_level(PIN_VALVE, valve_on ? 1 : 0);

        // ------------------- DRY DETECTION -----------------
        if (!dry_latched) {
            if (!cycle_active && psi >= CYCLE_START_PSI)
                cycle_active = true;

            if (cycle_active) {
                if (psi <= DRY_PRESSURE_MAX) {
                    if (dry_candidate_start == 0)
                        dry_candidate_start = now;
                    if ((now - dry_candidate_start) >= (int64_t)DRY_TIME_MS)
                        dry_latched = true;
                } else {
                    dry_candidate_start = 0;
                }
            }
        }

        // ------------------- HEATER CONTROL ----------------
        bool  heater_on = false;
        float duty      = 0.0f;

        if (!dry_latched && temp_valid) {
            float error = SETPOINT_C - temp_ema;
            duty = (temp_ema >= SETPOINT_C + HYST_C) ? 0.0f : KP * error;
            if (duty < 0.0f) duty = 0.0f;
            if (duty > 1.0f) duty = 1.0f;

            if ((now - window_start) >= (int64_t)WINDOW_MS)
                window_start = now;

            heater_on = ((now - window_start) < (int64_t)(duty * WINDOW_MS));
        }

        set_heater(heater_on);

        // ------------------- LOG OUTPUT --------------------
        ESP_LOGI(TAG,
            "P=%.2fpsi | Valve=%s | Heater=%s | BJT=%s | DRY=%s | T=%s | Duty=%.0f%%",
            psi,
            valve_on    ? "ON"   : "OFF",
            heater_on   ? "ON"   : "OFF",
            gpio_get_level(PIN_HEATER) ? "HIGH" : "LOW",
            dry_latched ? "YES"  : "NO",
            temp_valid  ? (char[16]){0}  : "NA",
            duty * 100.0f
        );

        if (temp_valid) {
            ESP_LOGI(TAG, "  T_ema=%.1f°C", temp_ema);
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

// ===============================================================
// --------------------------- ENTRY POINT -----------------------
// ===============================================================
void app_main(void) {
    init_spi();
    init_adc();
    init_gpio();

    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
}
