# Current Status

Phase 1 (conductive heating prototype) is complete. All experiment data, firmware, schematics, PCB design files, and progress reports are in the [`Conductive Heating Prototype/`](https://github.com/PubInv/Biochar-Toilet/tree/main/Project%20Files/Conductive%20Heating%20Prototype) folder.

We're now in Phase 2: induction heating using a magnetite matrix. The shift from resistive heating matters because induction generates heat directly inside the charring cup — no exposed coils corroding in the process environment, and faster energy delivery to the material itself.

Ongoing experiment logs are in the [Progress Reports](https://github.com/PubInv/Biochar-Toilet/blob/main/Project%20Files/Progress%20Reports.md).

<img width="849" height="482" alt="Screenshot 2025-11-21 at 6 10 39 PM" src="https://github.com/user-attachments/assets/6e3a36fb-60a5-46c5-8a6b-8c2908af25fd" />

---

# Biochar Toilet

A reliable toilet that could quickly reduce human fecal matter to completely sanitary biochar would make the poorest people of the
Earth healthier. In many communities, Western-style sewage systems would be very expensive and impractical due to
geology, the lack of density in farming communities made of small land holdings, a lack of capital, and a lack of
social and political organization to dig trenches and lay pipes efficiently.

Reducing a typical human stool to biochar requires an energy input of about 600 kilojoules (see detailed analysis in referenced spreadsheet).
Even for the poorest people, the cost of this electricity is quite low---less than one US cent $0.01USD. One kilowatt hour is 3.6 MJ (megajoules).
Expensive electrical power around the world is $0.45/kWh is $0.075 USD--or less than eight cents. Even if our toilet is inefficient, it
seems possible to make a toilet whose operating cost is affordable for the poorest people, at $27.375 per year (this cost does not include
the capital costs of the toilet).

Biochar can be used as a fuel or a garden amendment. Note, however, that human stools are so small (128 grams nominally) that this is not a meaningful approach for carbon sequestration.

# Initial Design Idea

The problem of toilets is a widely researched area. Our approach may be slightly novel. We start from some assumptions:

1. Electricity is an input.
2. Complete biological safety of the end product is required.
3. We choose to temporarily ignore the capital costs of the toilet in our research.
4. Because we have to char in the absence of oxygen to prevent combustion, we can assume that we need an air-tight or pressure controlled reaction vessel.
5. A major problem in charring is to first dry the sample, and drying is primarily a problem of water transport.

We therefore conclude that our basic design will be a sealed reaction vessel with a valve that allows steam to be transported out
of the reaction chamber. We will heat the chamber contents until a desired pressure is created and then transport the water
vapor away by opening the valve. The valve will be controlled so as to avoid most gases (air) from entering the chamber.
Opening the valve will "flash" the water in the sample into steam, which will have the desirable properties of being
extremely murderous to any organisms present in the sample, and physically destructive to both plant and animal cells.
It may even physically disrupt the sample, which could be advantageous.

When the sample is sufficiently dry, we will heat it to charring temperature. When the sample is fully charred, and then cooled
and possibly wetted to avoid spontaneous combustion, the operator will be alerted that the sample is safe to remove and either combust or use as a garden amendment.

# Energy Analysis

Volunteer Nupur Bhalla working with Invention Coach Robert L. Read has produced a [Google Sheet](https://docs.google.com/spreadsheets/d/1ZwIqP0B2wM6QrpZTNiK1hNnNmIEIal7qxYdgEJz8Iog/edit?usp=sharing) representing an energy analysis of the drying and heating process.

# Heating Mechanisms

Since we will have a sealed metal reaction vessel, radio frequency (RF) heating is a possibility.
However, it is expected to become less effective as the sample dries.
We therefore intend to explore the possibility of using simple Joule resistive heating of a stainless steel vessel directly.
RF heating, however, has the advantage of heating the sample volumetrically (not requiring conduction), which may be a tremendous advantage.

# Phase 1: Conductive Heating Prototype (Complete)

Phase 1 used a pressure cooker as the reaction vessel, a hot plate controlled by an SSR, and an ESP32-based sensing system. The goal was to confirm the core architecture before committing to higher-temperature methods.

It worked. When pressure drops below ~1 PSI while temperature keeps climbing, the system correctly identifies the dry state and cuts heater power automatically. Flash decompression also checked out — opening the solenoid under pressure causes trapped water to flash to steam. A bread charring test (~110°C, 2h40m) produced a visibly charred loaf, though the dryness algorithm didn't trigger since the bread's low moisture content didn't produce a distinct enough pressure signature during drying. Soot accumulation on the lid also flagged particulate management as something to design for in future iterations.

The hard limit we hit: full charring temperatures aren't safe inside a standard pressure cooker. Lawrence Kincheloe suggested placing an inner cup (heated by a silicon nitride igniter) inside the vessel so the outer shell stays cool — that idea feeds directly into Phase 2.

We also designed a custom control PCB for this phase (ESP32-H2, v0.3.0) with dual MOSFET valve drivers, dual SSR outputs, a pressure ADC, and a MAX31855 thermocouple interface. Everything is in the [`Conductive Heating Prototype/`](https://github.com/PubInv/Biochar-Toilet/tree/main/Project%20Files/Conductive%20Heating%20Prototype) folder.

# Phase 2: Induction System Development (In Progress)

We've switched from resistive heating to pulsed induction using a magnetite matrix. The magnetite sits inside the charring cup and converts the induction field directly into heat — no exposed coils, no conduction losses through the vessel wall.

Early results from Lawrence Kincheloe and Peter are encouraging. Five grams of magnetite reached 740°F in 60 seconds; the same mass of carbon steel bearings only hit 130°F. Volume-matching the steel to the magnetite closes the gap considerably (~940°F peak), which suggests surface area and density matter more than material choice at this scale. They also charred cheese puffs at 833°F over 16 minutes in open air, with residue on the crucible cap showing evidence of partial pyrolysis.

Full experiment logs are in the [Progress Reports](https://github.com/PubInv/Biochar-Toilet/blob/main/Project%20Files/Progress%20Reports.md).

# Research Plans

The next step is pushing the induction system to actual charring temperatures — hot enough to produce biochar from organic material comparable to human stool. Work on coil geometry and magnetic field focusing is ongoing in parallel with fabrication.

[![Basic Biochar Toilet Design](https://github.com/user-attachments/assets/ea41856b-7663-43a1-bf12-8a3a2311aef8)](https://github.com/user-attachments/assets/ea41856b-7663-43a1-bf12-8a3a2311aef8) 


# References

"An Experimental 13.56 MHz Radio Frequency Heating System
for Efficient Thermal Pretreatment of Wastewater Sludge",
Md. S. Ferdous, Ehssan H. Koupaie, Cigdem Eskicioglu, and Thomas Johnson*

# Volunteer Opportunities

This project is still active. If you have a background in mechanical engineering, heat engineering, or chemistry, we'd be glad to hear from you.

Reach out to Inventor Coach [Robert L. Read](https://github.com/PubInv/Biochar-Toilet/blob/main/read.robert@gmail.com) or Volunteer Coordinator [Miriam Castillo](https://www.linkedin.com/in/cstllgtrrz/). More at the [Volunteer Page](https://www.pubinv.org/volunteer/).
