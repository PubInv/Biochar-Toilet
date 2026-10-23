/**
 * @file main.c
 * @brief Bio-Char Control Board v0.1.0 — Dual Induction Heating & Integrated Control System
 *
 * Target:  ESP32-H2-DEVKITM-1-N4
 * SDK:     ESP-IDF v5.x
 *
 * Pin Configuration:
 *   GPIO0  → MAX31855 CLK
 *   GPIO1  → MAX31855 CS
 *   GPIO2  → MAX31855 DO (MISO — read-only, no MOSI needed)
 *   GPIO3  → Pressure Sensor ADC  (ADC1_CH2, 0–15 PSI via 0–5V voltage divider)
 *   GPIO4  → SSR 1 / Main Coil Induction Heater (active HIGH — drives BJT base)
 *   GPIO5  → SSR 2 / Catalyst Induction Heater  (active HIGH — drives BJT base)
 *   GPIO8  → WS2812 RGB LED                      (Status Indicator)
 *   GPIO10 → Solenoid Valve 1                    (MOSFET-driven)
 *   GPIO11 → Pump / Valve 2                      (MOSFET-driven, feature flagged)
 *   GPIO12 → Flush Button                        (Active LOW with internal pull-up)
 *
 * PIN CONFIGURATION NOTES:
 *   GPIO14/15 are reserved for 32 kHz RTC crystal on some variants.
 *   GPIO3 is shared with MTDI/JTAG.
 *
 * DUAL INDUCTION HEATING SYSTEM:
 *   - Main Coil Heater (SSR 1 / GPIO4): Temperature controlled via PWM/PID against SETPOINT_C.
 *   - Catalytic Converter Heater (SSR 2 / GPIO5): Runs continuously whenever cycle_active is true.
 *
 * PRESSURE SENSOR (0–15 PSI, 0–5V output):
 *   Mapped from ADC range (320 to 3008) across 0–15 PSI full scale.
 *
 * SAFETY HARDWARE INTERLOCKS:
 *   - Emergency overpressure limit (14.0 PSI): Instantly kills all heaters and latches solenoid open.
 *   - Emergency overtemp limit (250.0°C): Instantly kills all heaters and latches solenoid open.
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
#include "esp_cpu.h"
#include "soc/gpio_reg.h"

static const char *TAG = "biochar_induction";

// ===============================================================
// -------------------- MAX31855 THERMOCOUPLE --------------------
// ===============================================================
#define PIN_MAXCLK   GPIO_NUM_0    // MAX31855 Clock (SPI CLK)
#define PIN_MAXCS    GPIO_NUM_1    // MAX31855 Chip Select
#define PIN_MAXDO    GPIO_NUM_2    // MAX31855 Data Out (MISO — read-only)

// ===============================================================
// -------------------------- PINS --------------------------------
// ===============================================================
#define PIN_ADC_PRESSURE    ADC_CHANNEL_2   // GPIO3  — Pressure sensor (ADC1_CH2)
#define PIN_HEATER_MAIN     GPIO_NUM_4      // GPIO4  — SSR 1: Main Induction Coil
#define PIN_HEATER_CATALYST GPIO_NUM_5      // GPIO5  — SSR 2: Catalytic Converter Heater
#define PIN_LED_WS2812      GPIO_NUM_8      // GPIO8  — Onboard WS2812 RGB Status LED
#define PIN_VALVE           GPIO_NUM_10     // GPIO10 — Solenoid Valve 1 (MOSFET)
#define PIN_PUMP            GPIO_NUM_11     // GPIO11 — Pump (MOSFET, Solenoid Valve 2 driver)
#define PIN_FLUSH_BUTTON    GPIO_NUM_12     // GPIO12 — Manual Flush Button (Active LOW with internal pull-up)

// ===============================================================
// ------------------- FEATURE FLAGS -----------------------------
// ===============================================================
#define FEATURE_PUMP_ENABLED   0    // Set to 1 to enable automated pump control

// ===============================================================
// ------------------- PRESSURE CALIBRATION ----------------------
// ===============================================================
#define ADC_ZERO      320.0f    // ADC raw value at 0 PSI
#define ADC_FULL      3008.0f   // ADC raw value at 15 PSI (full scale)
#define PRESSURE_MAX  15.0f     // 15 PSI sensor full scale

// ===============================================================
// --------------------- SAFETY INTERLOCKS -----------------------
// ===============================================================
#define MAX_SAFE_PSI       14.0f   // Overpressure limit (kill heaters, latch valve open)
#define MAX_SAFE_TEMP_C   250.0f   // Overtemp limit (kill heaters, latch valve open)

// ===============================================================
// --------------------- LOGARITHMIC VALVE CONTROL ---------------
// ===============================================================
#define MIN_SEAL_PRESSURE  4.0f    // Hard floor to preserve seal integrity
#define TARGET_PRESSURE    8.0f    // Maximum target pressure before safety limits
#define TARGET_TEMP_C      121.0f  // Target temperature for sterilization release
#define VALVE_WINDOW_MS    2000    // Valve maximum burst duration (ms)
#define VALVE_COOLDOWN_MS  2000    // Mandatory cooldown duration between bursts (ms)

// ===============================================================
// ------------------- DRY DETECTION (PRESSURE) ------------------
// ===============================================================
#define CYCLE_START_PSI      0.30f      // Pressure must exceed this to start a cycle
#define REPRESSURIZE_TIMEOUT_MS  (5UL * 60UL * 1000UL) // 5 minutes timeout to drop target pressure
#define MAX_CYCLE_TIME_MS    (30UL * 60UL * 1000UL) // 30 minutes maximum cycle duration
#define LONG_PRESS_RESET_MS  3000LL     // 3 seconds long-press to reset emergency trip

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
typedef enum {
    STATE_OFF,
    STATE_HEATING,
    STATE_COOLING,
    STATE_ERROR
} system_state_t;

static system_state_t current_state = STATE_OFF;
static int negative_pressure_counter = 0;
static int64_t cooling_start_time = 0;
static float base_temp_for_runaway = -1.0f;
static float base_temp_for_absolute_runaway = -1.0f;

#define STAGNATION_WINDOW_SIZE 180
static float heating_temp_history[STAGNATION_WINDOW_SIZE];
static int64_t heating_time_history[STAGNATION_WINDOW_SIZE];
static int history_head = 0;
static int history_count = 0;
static int64_t last_history_record_time = 0;

static bool    valve_on            = false;
static bool    pump_on             = false;
static bool    cycle_active        = false;
static bool    dry_latched         = false;
static bool    has_pressurized     = false;
static bool    emergency_tripped   = false;
static int64_t dry_candidate_start = 0;
static int64_t cycle_start_time    = 0;
static int64_t button_press_start  = 0;
static bool    button_was_pressed  = false;
static bool    long_press_handled  = false;
static float   temp_ema            = NAN;
static int64_t window_start        = 0;
static int64_t valve_window_start  = 0;

static spi_device_handle_t       max31855_handle;
static adc_oneshot_unit_handle_t adc_handle;

// ===============================================================
// -------------------------- HELPERS ----------------------------
// ===============================================================
static float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static float adc_to_psi(int adc_raw) {
    return fmap((float)adc_raw, ADC_ZERO, ADC_FULL, 0.0f, PRESSURE_MAX);
}

static void set_main_heater(bool on) {
    // Active HIGH: GPIO4 HIGH → BJT base HIGH → collector completes SSR circuit → SSR ON
    gpio_set_level(PIN_HEATER_MAIN, (on && !emergency_tripped) ? 1 : 0);
}

static void set_catalyst_heater(bool on) {
    // Active HIGH: GPIO5 HIGH → BJT base HIGH → collector completes SSR circuit → SSR ON
    gpio_set_level(PIN_HEATER_CATALYST, (on && !emergency_tripped) ? 1 : 0);
}

static void set_pump(bool on) {
    pump_on = on;
    gpio_set_level(PIN_PUMP, on ? 1 : 0);
}

// ===============================================================
// -------------------- WS2812 RGB LED DRIVER --------------------
// ===============================================================
static void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t grb[3] = {g, r, b};
    uint32_t mask = (1UL << PIN_LED_WS2812);

    portDISABLE_INTERRUPTS();
    for (int i = 0; i < 3; i++) {
        uint8_t byte = grb[i];
        for (int bit = 7; bit >= 0; bit--) {
            if (byte & (1 << bit)) {
                // Bit 1: HIGH ~800ns (77 cycles at 96MHz), LOW ~350ns (34 cycles)
                REG_WRITE(GPIO_OUT_W1TS_REG, mask);
                uint32_t start = esp_cpu_get_cycle_count();
                while ((esp_cpu_get_cycle_count() - start) < 77) { __asm__ __volatile__("nop"); }

                REG_WRITE(GPIO_OUT_W1TC_REG, mask);
                start = esp_cpu_get_cycle_count();
                while ((esp_cpu_get_cycle_count() - start) < 34) { __asm__ __volatile__("nop"); }
            } else {
                // Bit 0: HIGH ~350ns (34 cycles at 96MHz), LOW ~800ns (77 cycles)
                REG_WRITE(GPIO_OUT_W1TS_REG, mask);
                uint32_t start = esp_cpu_get_cycle_count();
                while ((esp_cpu_get_cycle_count() - start) < 34) { __asm__ __volatile__("nop"); }

                REG_WRITE(GPIO_OUT_W1TC_REG, mask);
                start = esp_cpu_get_cycle_count();
                while ((esp_cpu_get_cycle_count() - start) < 77) { __asm__ __volatile__("nop"); }
            }
        }
    }
    portENABLE_INTERRUPTS();

    // WS2812 Reset pulse > 50us (5000 cycles)
    REG_WRITE(GPIO_OUT_W1TC_REG, mask);
    uint32_t start = esp_cpu_get_cycle_count();
    while ((esp_cpu_get_cycle_count() - start) < 5000) { __asm__ __volatile__("nop"); }
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
    // Configure outputs
    gpio_config_t out_conf = {
        .pin_bit_mask  = (1ULL << PIN_VALVE)           |
                         (1ULL << PIN_PUMP)            |
                         (1ULL << PIN_HEATER_MAIN)     |
                         (1ULL << PIN_HEATER_CATALYST) |
                         (1ULL << PIN_LED_WS2812),
        .mode          = GPIO_MODE_OUTPUT,
        .pull_up_en    = GPIO_PULLUP_DISABLE,
        .pull_down_en  = GPIO_PULLDOWN_ENABLE,   // Pull-down keeps drivers off at boot
        .intr_type     = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&out_conf));

    gpio_set_level(PIN_VALVE,           0);   // Solenoid Valve 1 OFF
    gpio_set_level(PIN_PUMP,            0);   // Pump / Valve 2 OFF
    gpio_set_level(PIN_HEATER_MAIN,     0);   // SSR 1 Main Coil OFF
    gpio_set_level(PIN_HEATER_CATALYST, 0);   // SSR 2 Catalyst Heater OFF
    gpio_set_level(PIN_LED_WS2812,      0);   // WS2812 LED pin LOW

    set_led_color(0, 0, 0);                   // WS2812 reset

    // Configure Flush Button input
    gpio_config_t btn_conf = {
        .pin_bit_mask  = (1ULL << PIN_FLUSH_BUTTON),
        .mode          = GPIO_MODE_INPUT,
        .pull_up_en    = GPIO_PULLUP_ENABLE,     // Pull-up resistor (Active LOW button press to GND)
        .pull_down_en  = GPIO_PULLDOWN_DISABLE,
        .intr_type     = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn_conf));
}

// ===============================================================
// ------------------------- MAIN TASK ---------------------------
// ===============================================================
static void transition_to(system_state_t new_state, float current_temp, int64_t now) {
    if (current_state == STATE_ERROR) return;

    current_state = new_state;

    if (new_state == STATE_COOLING || new_state == STATE_OFF) {
        cooling_start_time = now;
    }

    if (new_state == STATE_OFF || new_state == STATE_COOLING) {
        base_temp_for_absolute_runaway = current_temp;
        base_temp_for_runaway = current_temp;
    }

    if (new_state == STATE_HEATING) {
        history_head = 0;
        history_count = 0;
        last_history_record_time = 0;
        if (!isnan(current_temp)) {
            heating_temp_history[0] = current_temp;
            heating_time_history[0] = now;
            history_head = 1;
            history_count = 1;
            last_history_record_time = now;
        }
    }

    if (new_state == STATE_ERROR) {
        emergency_tripped = true;
        ESP_LOGE(TAG, "🚨 SYSTEM ENTERED ERROR STATE: All heaters DISABLED, valve FORCED OPEN!");
    }
}

static void control_task(void *arg) {
    ESP_LOGI(TAG, "╔════════════════════════════════════════════════╗");
    ESP_LOGI(TAG, "║ SYSTEM READY: DUAL INDUCTION CONTROL SYSTEM    ║");
    ESP_LOGI(TAG, "║ - Pressure Control + Valve 1 (0-15 PSI Sensor) ║");
    ESP_LOGI(TAG, "║ - SSR 1: Main Coil Induction Heater            ║");
    ESP_LOGI(TAG, "║ - SSR 2: Catalyst Induction Heater             ║");
    ESP_LOGI(TAG, "║ - Solenoid Valve 2 / Pump Control              ║");
    ESP_LOGI(TAG, "║ - Safety Interlocks: P > 14PSI / T > 250°C     ║");
    ESP_LOGI(TAG, "╚════════════════════════════════════════════════╝");
    ESP_LOGI(TAG, "Pin map:");
    ESP_LOGI(TAG, "  GPIO0  → MAX31855 CLK");
    ESP_LOGI(TAG, "  GPIO1  → MAX31855 CS");
    ESP_LOGI(TAG, "  GPIO2  → MAX31855 DO");
    ESP_LOGI(TAG, "  GPIO3  → Pressure Sensor ADC (0-15 PSI)");
    ESP_LOGI(TAG, "  GPIO4  → SSR 1 (Main Induction Coil)");
    ESP_LOGI(TAG, "  GPIO5  → SSR 2 (Catalyst Induction Heater)");
    ESP_LOGI(TAG, "  GPIO8  → WS2812 RGB LED Status Indicator");
    ESP_LOGI(TAG, "  GPIO10 → Solenoid Valve 1");
    ESP_LOGI(TAG, "  GPIO11 → Solenoid Valve 2 / Pump");
    ESP_LOGI(TAG, "  GPIO12 → Flush Button (Active LOW)");

    window_start = esp_timer_get_time() / 1000LL;
    valve_window_start = window_start;

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

        // ------------------- SAFETY INTERLOCKS -----------------
        if (current_state != STATE_ERROR) {
            // Lazy initialization of base temps
            if (current_state == STATE_OFF || current_state == STATE_COOLING) {
                if (base_temp_for_absolute_runaway < 0.0f && temp_valid) {
                    base_temp_for_absolute_runaway = temp_ema;
                }
                if (base_temp_for_runaway < 0.0f && temp_valid) {
                    base_temp_for_runaway = temp_ema;
                }
            }

            // 1. Negative Pressure Failure
            if (psi < -0.2f) {
                negative_pressure_counter++;
                if (negative_pressure_counter >= 3) {
                    ESP_LOGE(TAG, "CRITICAL ERROR: Negative pressure detected (%.2f PSI)", psi);
                    valve_on = true; // Vent valve OPEN
                    transition_to(STATE_ERROR, temp_ema, now);
                }
            } else {
                negative_pressure_counter = 0;
            }

            // 2. Unintended Temperature Rise
            if ((current_state == STATE_OFF || current_state == STATE_COOLING) && temp_valid) {
                // ABSOLUTE 50C RUNAWAY: TRIPS IMMEDIATELY, NO GRACE PERIOD
                if (base_temp_for_absolute_runaway >= 0.0f) {
                    if (temp_ema - base_temp_for_absolute_runaway >= 50.0f) {
                        ESP_LOGE(TAG, "CRITICAL ERROR: Absolute thermal runaway (+50C in OFF/COOLING)");
                        transition_to(STATE_ERROR, temp_ema, now);
                    } else if (temp_ema < base_temp_for_absolute_runaway) {
                        base_temp_for_absolute_runaway = temp_ema;
                    }
                }

                // 5C STUCK BJT RUNAWAY: SUPPRESSED BY 1 HOUR GRACE PERIOD
                if (base_temp_for_runaway >= 0.0f) {
                    bool grace = ((current_state == STATE_COOLING || current_state == STATE_OFF) && cooling_start_time > 0 && (now - cooling_start_time) <= 3600000);
                    if (!grace) {
                        if (temp_ema - base_temp_for_runaway >= 5.0f) {
                            ESP_LOGE(TAG, "CRITICAL ERROR: Stuck BJT thermal runaway (+5C)");
                            transition_to(STATE_ERROR, temp_ema, now);
                        } else if (temp_ema < base_temp_for_runaway) {
                            base_temp_for_runaway = temp_ema;
                        }
                    }
                }
            }

            // 3. Stagnation in Heating
            if (current_state == STATE_HEATING && temp_valid) {
                // Rate limit recording to ~20 seconds (1 hour window across 180 samples)
                if (now - last_history_record_time >= 20000) {
                    heating_temp_history[history_head] = temp_ema;
                    heating_time_history[history_head] = now;
                    last_history_record_time = now;
                    history_head = (history_head + 1) % STAGNATION_WINDOW_SIZE;
                    if (history_count < STAGNATION_WINDOW_SIZE) history_count++;
                }

                int oldest_idx = history_count == STAGNATION_WINDOW_SIZE ? history_head : 0;

                while (history_count > 1 && (now - heating_time_history[oldest_idx]) > 3600000) {
                    oldest_idx = (oldest_idx + 1) % STAGNATION_WINDOW_SIZE;
                    history_count--;
                }

                if (history_count > 1 && (now - heating_time_history[oldest_idx]) >= 3595000) {
                    float oldest_temp = heating_temp_history[oldest_idx];
                    if (temp_ema - oldest_temp < 3.0f) {
                        ESP_LOGE(TAG, "CRITICAL ERROR: Heater stagnation (temp rise < 3C over 1 hour)");
                        transition_to(STATE_ERROR, temp_ema, now);
                    }
                }
            }

            // Sync states based on active cycle
            if (cycle_active && !dry_latched) {
                if (current_state != STATE_HEATING) transition_to(STATE_HEATING, temp_ema, now);
            } else if (current_state == STATE_COOLING) {
                if (current_state != STATE_COOLING) transition_to(STATE_COOLING, temp_ema, now);
            } else {
                if (current_state != STATE_OFF) transition_to(STATE_OFF, temp_ema, now);
            }
        }

        // ------------------- EMERGENCY TRIP CHECK ----------
        if (psi >= MAX_SAFE_PSI || (temp_valid && temp_ema >= MAX_SAFE_TEMP_C)) {
            if (!emergency_tripped) {
                emergency_tripped = true;
                ESP_LOGE(TAG, "🚨 EMERGENCY TRIP: Over-limit detected! P=%.2f PSI, T=%.1f°C", psi, temp_ema);
            }
        }

        // ------------------- LOGARITHMIC VALVE CONTROL -----------------
        static bool release_active = false;
        static int64_t burst_start_time = 0;
        static int64_t burst_duration = 0;
        static int64_t cooldown_start_time = -VALVE_COOLDOWN_MS;
        static float dynamic_target_pressure = TARGET_PRESSURE;
        static int64_t repressurize_timer_start = 0;
        static bool is_repressurizing = false;

        float vent_lower_bound = dynamic_target_pressure - 2.0f;
        if (vent_lower_bound < MIN_SEAL_PRESSURE) {
            vent_lower_bound = MIN_SEAL_PRESSURE;
        }

        if (current_state == STATE_ERROR) {
            valve_on = true;  // Latch valve OPEN in emergency trip
            release_active = false;
            is_repressurizing = false;
        } else {
            // ------------------- DRYNESS & DYNAMIC TARGET LOGIC -------------------
            if (!valve_on && !release_active && !dry_latched && cycle_active) {
                if (temp_valid && temp_ema >= 100.0f && psi < dynamic_target_pressure) {
                    if (!is_repressurizing) {
                        is_repressurizing = true;
                        repressurize_timer_start = now;
                    } else {
                        if ((now - repressurize_timer_start) >= (int64_t)REPRESSURIZE_TIMEOUT_MS) {
                            if (dynamic_target_pressure > MIN_SEAL_PRESSURE) {
                                dynamic_target_pressure -= 1.0f;
                                if (dynamic_target_pressure < MIN_SEAL_PRESSURE) {
                                    dynamic_target_pressure = MIN_SEAL_PRESSURE;
                                }
                                ESP_LOGI(TAG, "⏱️ TIMEOUT: Dropped target pressure to %.1f PSI", dynamic_target_pressure);
                                repressurize_timer_start = now;
                                vent_lower_bound = dynamic_target_pressure - 2.0f;
                                if (vent_lower_bound < MIN_SEAL_PRESSURE) {
                                    vent_lower_bound = MIN_SEAL_PRESSURE;
                                }
                            } else {
                                dry_latched = true;
                                is_repressurizing = false;
                                cycle_active = false;
                                cycle_start_time = 0;
                                ESP_LOGI(TAG, "🌱 DRY LATCHED: Failed to reach minimum seal pressure. Biochar drying stage complete!");
                            }
                        }
                    }
                } else {
                    is_repressurizing = false;
                }
            } else {
                is_repressurizing = false;
            }

            // Trigger release cycle only when both pressure and temp targets are met
            if (!release_active && psi >= dynamic_target_pressure && temp_valid && temp_ema >= TARGET_TEMP_C) {
                release_active = true;
                ESP_LOGI(TAG, "💨 VALVE: Target %.1f PSI reached. Initiating release.", dynamic_target_pressure);
                if (is_repressurizing) {
                    ESP_LOGI(TAG, "📈 REPRESSURIZE: Time to target was %d ms", (int)(now - repressurize_timer_start));
                }
            }

            if (release_active) {
                // CRITICAL SAFETY FLOOR: If we drop to or below vent lower bound, close completely and end release
                if (psi <= vent_lower_bound) {
                    valve_on = false;
                    release_active = false;
                    ESP_LOGI(TAG, "💨 VALVE: Dropped below vent bound %.1f PSI. Closing.", vent_lower_bound);
                } else {
                    if (valve_on) {
                        // We are in an active burst
                        if ((now - burst_start_time) >= burst_duration) {
                            // Burst finished, close valve and start cooldown
                            valve_on = false;
                            cooldown_start_time = now;
                        }
                    } else {
                        // Valve is closed. Are we in cooldown?
                        if ((now - cooldown_start_time) >= (int64_t)VALVE_COOLDOWN_MS) {
                            // Cooldown finished, start a new burst
                            float clamped_psi = psi > dynamic_target_pressure ? dynamic_target_pressure : psi;
                            float range_p = dynamic_target_pressure - vent_lower_bound;
                            float duty_cycle = 0.0f;

                            if (range_p > 0.0f) {
                                float t = (clamped_psi - vent_lower_bound) / range_p;
                                float log_factor = logf(1.0f + (t * 1.71828f)) / 1.0f;
                                duty_cycle = log_factor;
                                if (duty_cycle < 0.0f) duty_cycle = 0.0f;
                                if (duty_cycle > 1.0f) duty_cycle = 1.0f;
                            }

                            burst_duration = (int64_t)(duty_cycle * VALVE_WINDOW_MS);
                            burst_start_time = now;
                            valve_on = true;
                        }
                    }
                }
            } else {
                // Keep valve closed while not releasing
                valve_on = false;
            }
        }
        gpio_set_level(PIN_VALVE, valve_on ? 1 : 0);

        // ------------------- FLUSH BUTTON & EMERGENCY RESET -
        bool button_pressed = (gpio_get_level(PIN_FLUSH_BUTTON) == 0); // Active LOW
        if (button_pressed) {
            if (!button_was_pressed) {
                button_was_pressed = true;
                button_press_start = now;
                long_press_handled = false;
            } else {
                int64_t hold_duration = now - button_press_start;
                if (hold_duration >= LONG_PRESS_RESET_MS && !long_press_handled) {
                    long_press_handled = true;
                    if (emergency_tripped) {
                        if (psi < MAX_SAFE_PSI && (!temp_valid || temp_ema < MAX_SAFE_TEMP_C)) {
                            emergency_tripped = false;
                            current_state = STATE_OFF;
                            negative_pressure_counter = 0;
                            base_temp_for_runaway = -1.0f;
                            base_temp_for_absolute_runaway = -1.0f;
                            valve_on = false;
                            dry_latched = false;
                            cycle_active = false;
                            dynamic_target_pressure = TARGET_PRESSURE;
                            ESP_LOGI(TAG, "✅ EMERGENCY TRIP RESET: Conditions safe. System returned to IDLE!");
                        } else {
                            ESP_LOGE(TAG, "⚠️ CANNOT RESET TRIP: System conditions still unsafe! P=%.2f PSI, T=%.1f°C", psi, temp_ema);
                        }
                    } else if (cycle_active) {
                        cycle_active = false;
                        dry_latched = false;
                        has_pressurized = false;
                        dry_candidate_start = 0;
                        cycle_start_time = 0;
                        current_state = STATE_OFF;
                        ESP_LOGI(TAG, "🛑 CYCLE CANCELLED: Biochar cycle manually cancelled via long press!");
                    }
                }
            }
        } else {
            if (button_was_pressed) {
                int64_t hold_duration = now - button_press_start;
                button_was_pressed = false;

                if (!long_press_handled && hold_duration < LONG_PRESS_RESET_MS) {
                    if (!cycle_active && !emergency_tripped) {
                        cycle_active = true;
                        cycle_start_time = now;
                        dry_latched = false;
                        dynamic_target_pressure = TARGET_PRESSURE;
                        ESP_LOGI(TAG, "🚽 FLUSH BUTTON PRESSED: Biochar cycle initiated manually!");
                    }
                }
            }
        }

        // Pressure-based auto cycle start (if not already started)
        if (!cycle_active && !emergency_tripped && psi >= CYCLE_START_PSI) {
            cycle_active = true;
            cycle_start_time = now;
            dry_latched = false;
            dynamic_target_pressure = TARGET_PRESSURE;
            ESP_LOGI(TAG, "🔥 CYCLE STARTED: Pressure exceeded threshold (%.2f PSI)", psi);
        }

        // ------------------- CYCLE TIMEOUT ---
        if (cycle_active) {
            int64_t cycle_elapsed = now - cycle_start_time;

            // Check 30-minute max cycle timeout
            if (cycle_elapsed >= (int64_t)MAX_CYCLE_TIME_MS) {
                ESP_LOGI(TAG, "⏰ CYCLE COMPLETE: Reached maximum cycle duration (30 mins). Resetting to IDLE.");
                cycle_active = false;
                dry_latched = false;
                cycle_start_time = 0;
            }
        }

        // ------------------- MAIN HEATER CONTROL -----------
        bool  main_heater_on = false;
        float duty           = 0.0f;

        if (cycle_active && !dry_latched && !emergency_tripped && temp_valid) {
            float error = SETPOINT_C - temp_ema;
            duty = (temp_ema >= SETPOINT_C + HYST_C) ? 0.0f : KP * error;
            if (duty < 0.0f) duty = 0.0f;
            if (duty > 1.0f) duty = 1.0f;

            if ((now - window_start) >= (int64_t)WINDOW_MS)
                window_start = now;

            main_heater_on = ((now - window_start) < (int64_t)(duty * WINDOW_MS));
        }

        set_main_heater(main_heater_on);

        // ------------------- CATALYST HEATER CONTROL -------
        // Catalytic converter heater runs continuously whenever cycle_active is true
        // and dry state has not latched.
        bool catalyst_heater_on = cycle_active && !dry_latched && !emergency_tripped;
        set_catalyst_heater(catalyst_heater_on);

        // ------------------- PUMP CONTROL ------------------
#if FEATURE_PUMP_ENABLED
        // Example logic when enabled: run pump whenever main heater is active
        set_pump(main_heater_on && !emergency_tripped);
#else
        set_pump(false);
#endif

        // ------------------- WS2812 LED STATUS INDICATOR ---
        static bool flash_state = false;
        static int64_t last_flash_toggle = 0;
        if (now - last_flash_toggle >= 500) {
            flash_state = !flash_state;
            last_flash_toggle = now;
        }

        if (emergency_tripped) {
            // RED Flashing
            if (flash_state) set_led_color(255, 0, 0);
            else             set_led_color(0, 0, 0);
        } else if (current_state == STATE_HEATING) {
            // ORANGE/YELLOW
            set_led_color(255, 120, 0);
        } else if (dry_latched) {
            // BLUE
            set_led_color(0, 0, 255);
        } else {
            // GREEN (IDLE / Ready)
            set_led_color(0, 255, 0);
        }

        // ------------------- LOG OUTPUT --------------------
        ESP_LOGI(TAG,
            "P=%.2fpsi | Valve=%s | Pump=%s | MainHeater=%s | CatHeater=%s | TRIP=%s | DRY=%s | CycleActive=%s | Duty=%.0f%%",
            psi,
            valve_on           ? "OPEN" : "CLOSED",
            pump_on            ? "ON"   : "OFF",
            main_heater_on     ? "ON"   : "OFF",
            catalyst_heater_on ? "ON"   : "OFF",
            emergency_tripped  ? "YES"  : "NO",
            dry_latched        ? "YES"  : "NO",
            cycle_active       ? "YES"  : "NO",
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
