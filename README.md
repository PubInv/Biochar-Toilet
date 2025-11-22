# Current Status

As of Nov. 21, our EE Hardhik Pyla has our prototype equipment, which consists of a pressure cooker.
He has calibrated the pressure sensor. He has a small bug controlling the valve.  
The thermocouple works.  He is going to develop a simple algorithm for detecting when the sample is dry based on temperature and pressure. (Basically, temperature rises without a pressure rise when the sample is dry.)

After that we will use a solid-state relay to control a hot plate to turn it off when we we are "dry".
This will require supervision at first.

We do not believe it is safe to char inside pressure cooker. Lawrence Kincheloe came up with a good
idea of using a silicon nitride igniter as a heat source inside a metal or ceramic cup inside the pressure cooker.
The outside of the pressure cooker can be kept cool. If our wires will survive this level of heat, then we can char.
We believe having an electric biochar producer will be valuable as a means of testing the disposition of the dangerous
syngases produced by the charring process. Even though this containment vessel is obviously inadequate for the final purpose of producing a toilet, we believe this will be progress.


We have made a [spreadsheet model](https://docs.google.com/spreadsheets/d/1ZwIqP0B2wM6QrpZTNiK1hNnNmIEIal7qxYdgEJz8Iog/edit?usp=sharing) of "flash" decompression.

One of our volunteers, Hardhik, has begun prototype a flash system. We need someone to design a reaction vessel.

# Biochar Toilet

A reliable toilet that could quickly reduce human fecal matter to completely sanitary biochar would make the pooreset people of the
Earth healthier. In many communities, Western-style sewage systems would be very expensive, and impractical for other reasons,
such as geology, the lack of density in farming communities made of small land holdings, a lack of capital, and a lack of
social and political organization able to dig the trenches and lay pipes efficiently.

Reducing a typical human stool to biochar requires an energy input of about 600 kilojoules (see detailed analysis in referenced spreadsheet).
Even for the poorest people, the cost of this electricity is quite low---less than one US cent \$0.01USD. One kilowatt hour is 3.6 MJ (megajoules).
Expensive electrical power around the world is $0.45/kWh is $0.075 USD--or less than eight cents. Even if our toilet is inefficient, it
seems possible to make toilet whose operating cost is affordable for the poorest persons, at $27.375 per year (this cost does not include
the capital costs of the toilet.)

Biochar can be used a fuel or a garden amendment. Note, however, that human stools are so small (128 grams nominally) that this is not an meaninful approach for carbon sequestration.


# Initial Design Idea
The problem of toilets is a widely researched area. Our approach may be slightly novel. We start from some assumptions:
1. Electicity is an input.
1. Complete biological safety of the endproduct is required.
1. We choose to temporarily ignore the capital costs of the toilet in our research.
1. Because we have to char in the absence of oxygen to prevent combustion, we can assume that we need an air-tight or pressure controlled
    reaction vessel.
1. A major problem in charring is to first dry the sample, and drying is primarily a problem of water transport.

We therefore conclude our basic design will be a sealed reaction vessel with a valve that allow steam to be transported out
of the reaction chamber. We will heat the chamber contents until a desired pressure is created and then transport the water
vapor away by opening the valve. The valve will be controlled so as to avoid most gases (air) from enterring the chamber.
Opening the valve will "flash" the water in the sample into steam, which will have the desirable properties of being
extremely murderous to any organisms present in the sample, and physically destructive to both plant and animal cells.
Possibly it will even physically disrupt the sample, which may be advantageous.

When the sample is sufficiently dry, we will heat it to charring temperature. When the sample is fully charred, and then cooled
and possibly wetted to avoid spontaneous combustion, the
operator will be alerted that the sample is safe to remove and either combust or use as a garden amendment.

# Energy Analysis

Volunter Nupur Bhalla working with Invention Coach Robert L. Read has produced a [Google Sheet](https://docs.google.com/spreadsheets/d/1ZwIqP0B2wM6QrpZTNiK1hNnNmIEIal7qxYdgEJz8Iog/edit?usp=sharing) represnting an energy analysis of the drying and heating process.



# Heating Mechanisms
Since we will have a sealed metal reaction vessel, radio frequency (RF) heating is possibility.
However, it is expected to become less effective as the sample dries.
We therefore intend to explore the possibility of using simple Joule resistive heating of a stainless steel vessel directly.
RF heating, however, has the advantage of heating the sample volumetric (not require conduction), which may be a tremendouse advantage.

# Phase 1

Having gotten a new volunteer who wants to build an embedded system, we propose a Phase 1 as a "spike" prototype to test out the basic principles.
In this phase, the goal is to build the smallest, safest system that we can. 
We imagine making a "reaction vessel" which is very small---perhaps 50 ml, large enough to dry a piece of fruit as a test.

As Nupur Bhalla and I have demonstrated in the energy analysis spreadsheet, the act of producing char can be divided into two operations:
drying to low water content, and then raising the temperature sufficiently to produce char (all in the absence of additionaly O2 to avoid
combustion.) The Phase 1 system needs to raise the temperature enough to produce a pressure in the chamber of 2 atmospheres (or possibly 
a lower pressure for testing), but if the sample has a high water content, this will likely occur at just a little above the boiling point 
of water (100C, or a little higher under pressure.) The Phase 1 system does not need to operate above 150C, which will make it slightly safer.

When the solenoid valve is opened, the pressure drop will instantly lower the boiling point of water, causing the water to "flash" into
steam. This has the advantage that it "destroy" the sample. For example, a grape with an intact skin would literally be expected to "explode". 
This will likely be messy in the chamber. However, this is NOT bad for treating a human stool---it has the advantage of being very 
destructive of intact cells, including bacterian and plant cells that are in the stool, for example.

However, opening the solenoid valve briefly is likely to spray hot, scalding steam out the valve. It is essential for saftey that this steam enter
a separate containment chamber. The purpose of the containment chamber is allow the offgases to cool (probably liquefying). The containment 
chamber should be close to atmospheric pressure, probably open to the atmospher. An odor-reduing carbon filter may connect the containment 
chamber to the outside air, but that is not necessary for Phase 1---but the safety concerns are.

For that reason, an emergency pop-off valve needs to be build into the Phase 1 reaction vessel. A software error or someother error could 
easily allow the recation vessel to develop high pressure which would be dangerous.  However, a lot of the software and machinery 
design could be accomplished with a reaction vessel which is simply not tightly closed. Although this will not allow the "flashing" of
the steam, in other ways the Phase 1 machine could be tested.

If the reaction vessel is not sealed, then it could be constructed in many differnt ways---for example, with a babyfood jar of a Mason jar,
or even a cardboard box. Since nylon melts at about 230C, it would even be possible to 3D print a nylon reaction vessel for Phase 1. (Nylon
would not survive the 400C temperatures needed for charring.)

I imagine a BOM for the Phase 1 system to be:
1. An I2C pressure sensor with a range of at least 2.5 Atms.
2. A heating element, such as a power resistor. (Two of these in series might work: [https://www.digikey.com/en/products/detail/te-connectivity-passive-product/HSC1001R0J/2055297?_gl=1*15lsl1x*_up*MQ..&gclid=Cj0KCQjwzYLABhD4ARIsALySuCTm8nkVdOfNr_GjOg-3iFiSH7pkuDwZ5D46byvvL9cms07OnCRo0-UaAs2xEALw_wcB&gclsrc=aw.ds](https://www.digikey.com/en/products/detail/te-connectivity-passive-product/HSC1001R0J/2055297?_gl=1*15lsl1x*_up*MQ..&gclid=Cj0KCQjwzYLABhD4ARIsALySuCTm8nkVdOfNr_GjOg-3iFiSH7pkuDwZ5D46byvvL9cms07OnCRo0-UaAs2xEALw_wcB&gclsrc=aw.ds).)
3. A digital thermometer (This operates up to 125C, which may be good enough for phase 1: [https://www.adafruit.com/product/642](https://www.adafruit.com/product/642)).
4. A solenoid valve that can open (and close) under micrconctroller control (Note, it is unclear if this is air-tight: [https://www.adafruit.com/product/996](https://www.adafruit.com/product/996)).
5. Either a relay, or relay break out board, or  motor controller, or a transistor with a fly-back diode to control the solenoid.
6. A transistor to control the power to the heater. [https://www.adafruit.com/product/355](https://www.adafruit.com/product/355).

# Flash System Prototype - Test 1

On Nov.7th an initial prototype was tested. This consisted of a pressure cooker containing a cup of water,
with a thermocouple added and a hose connected to an electronic valve, leading to a condenser.

This experiment was large confirmatory of our approach, but we learned certain things:
1. At high power and about 15 psi, the steam does not "flash" off. When the valve is opened, the
steam escapes, but it keeps flowing. Our valve apparently produces so much restriction that the
pressure does not drop quickly (though it does drop, but remains positive).
2. The temperature did rise DRASTICALLY when the water dried up. Our theory that we can detect this by the time that the pressure ceases to rise and the temperature does rise is well established.
3. A small amount of steam leaked through my high-temperature epoxy around the thermocouple.

So in principle, this was a successful test.

# Research Plans

We indent to continue theoretical research and design. Simultaneously, we intend to make a mini-scale (Phase 1) system, to test our assumptions
and calculations. That is, we intend to make a very small reaction vessel which can be temperature and pressure controlled, surmounted
with a controllable valve. We will test this system's ability to dry and char small samples of biological material, such as bread.

![Basic Biochar Toilet Design](https://github.com/user-attachments/assets/ea41856b-7663-43a1-bf12-8a3a2311aef8)

# References

"An Experimental 13.56 MHz Radio Frequency Heating System
for Efficient Thermal Pretreatment of Wastewater Sludge",
Md. S. Ferdous, Ehssan H. Koupaie, Cigdem Eskicioglu, and Thomas Johnson*


