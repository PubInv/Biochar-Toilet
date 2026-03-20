// --- PARAMETERS ---
$fn = 100;

// Pot Dimensions (Oval)
pot_width = 150;
pot_length = 180;
pot_height = 200;
wall_thick = 4;

// Catalyst Dimensions
cat_diam = 60;
cat_height = 100;

// --- MODULES ---

module oval_shape(w, l) {
    scale([1, l/w, 1]) circle(d=w);
}

module pressure_vessel() {
    color("Silver", 0.6)
    difference() {
        linear_extrude(pot_height)
            oval_shape(pot_width + wall_thick*2, pot_length + wall_thick*2);

        // Inner cavity
        translate([0,0, wall_thick])
        linear_extrude(pot_height + 1)
            oval_shape(pot_width, pot_length);

        // Side Steam Port (Hole) on the "Wide Part" (X-axis side)
        // Positioned near top rim
        translate([pot_width/2, 0, pot_height - 30])
        rotate([0, 90, 0])
        cylinder(h=wall_thick * 3, d=15);

        // NEW: Induction Coil Ports (Y-axis side)
        // Bottom Port Hole
        translate([0, pot_length/2, wall_thick + 5 + 20 + 2.5])
        rotate([-90, 0, 0])
        cylinder(h=wall_thick * 4, d=10);

        // Top Port Hole
        translate([0, pot_length/2, wall_thick + 5 + 20 + 6*25 + 2.5])
        rotate([-90, 0, 0])
        cylinder(h=wall_thick * 4, d=10);

        // NEW: Thermocouple Port (Opposite X-axis side)
        translate([-pot_width/2, 0, pot_height - 30])
        rotate([0, -90, 0])
        cylinder(h=wall_thick * 3, d=8);
    }

    // Thermocouple Fitting (Visual)
    translate([-pot_width/2 - wall_thick, 0, pot_height - 30])
    rotate([0, -90, 0]) {
        color("Gold") cylinder(h=10, d=12); // Hex nut
        color("Silver") cylinder(h=30, d=4); // Probe inside
    }
}

module internal_coil_liner() {
    // The "Bucket" that holds the waste and induction coils
    // Ceramic material (White)
    liner_w = pot_width - 5;
    liner_l = pot_length - 5;
    liner_h = pot_height - 60;

    // Coil parameters
    z_start = 20;
    spacing = 25;

    // Coil exit Y position (Stick out of +Y side)
    port_y = liner_l/2;

    translate([0,0, wall_thick + 5])
    union() {
        color("White", 0.4)
        difference() {
            // Main Body
            difference() {
                linear_extrude(liner_h)
                    oval_shape(liner_w, liner_l);
                translate([0,0, 10])
                linear_extrude(liner_h)
                    oval_shape(liner_w - 20, liner_l - 20);
            }

            // NEW: Holes for Coil Ports
            // Bottom
            translate([0, port_y, z_start + 2.5])
            rotate([-90, 0, 0])
            cylinder(h=20, d=8);

            // Top
            translate([0, port_y, z_start + 6*spacing + 2.5])
            rotate([-90, 0, 0])
            cylinder(h=20, d=8);
        }

        // Copper Coils Embedded in Wall
        color("Orange")
        union() {
            // Stacked Hollow Coils
            for(i = [0:6]) {
                translate([0,0, z_start + i*spacing])
                linear_extrude(5)
                difference() {
                    // Outer square
                    difference() {
                        oval_shape(liner_w - 5, liner_l - 5);
                        oval_shape(liner_w - 15, liner_l - 15);
                    }
                    // Inner hollow
                    difference() {
                        oval_shape(liner_w - 7, liner_l - 7);
                        oval_shape(liner_w - 13, liner_l - 13);
                    }
                }
            }

            // NEW: Ports (Legs) extending out
            // Bottom
            translate([0, port_y - 5, z_start + 2.5])
            rotate([-90, 0, 0])
            difference() {
                 cylinder(h=40, d=6);
                 translate([0,0,-1]) cylinder(h=42, d=3);
            }

            // Top
            translate([0, port_y - 5, z_start + 6*spacing + 2.5])
            rotate([-90, 0, 0])
            difference() {
                 cylinder(h=40, d=6);
                 translate([0,0,-1]) cylinder(h=42, d=3);
            }
        }
    }
}

module smart_lid_assembly() {
    // Positioned at top of pot
    translate([0,0, pot_height - 10]) {

        // 1. The Oval Lid Plate
        color("DimGray")
        linear_extrude(8)
            oval_shape(pot_width + 8, pot_length + 8);

        // 2. The High-Temp Silicone Gasket (Red)
        color("Red")
        translate([0,0,8])
        linear_extrude(4)
            difference() {
                oval_shape(pot_width + 8, pot_length + 8);
                oval_shape(pot_width - 10, pot_length - 10);
            }

        // 3. Electrical Feedthrough (REMOVED - now on side)

        // 4. NEW: Pressure Relief Valve
        // Placed near the "back" (Negative Y)
        translate([0, -pot_length/2 + 30, 8])
        union() {
            color("Gold") cylinder(h=10, d=12); // Valve Body
            color("Red") translate([0,0,10]) cylinder(h=5, d=14); // Cap
            color("Silver") translate([0,0,15]) cylinder(h=5, d=4); // Stem
        }
    }
}

module side_plumbing_assembly() {
    // Attached to the side port
    // Position matches the hole in pressure_vessel
    port_z = pot_height - 30;
    port_x = pot_width/2 + wall_thick;

    translate([port_x, 0, port_z]) {

        // 1. Gasket & Flange
        rotate([0, 90, 0]) {
            color("Red") cylinder(h=2, d=30); // Silicone Gasket
            translate([0,0,2]) color("Silver") cylinder(h=5, d=40); // Flange Plate
        }

        // 2. Compression Fitting & Elbow
        translate([7, 0, 0]) { // 2mm gasket + 5mm flange
            rotate([0, 90, 0]) color("Silver") cylinder(h=15, d=18); // Fitting

            // 3. Solenoid Valve Block
            // Mounted at end of fitting
            translate([15 + 15, 0, 0]) { // Fitting length + half block width
                color("Blue") cube([30,30,30], center=true);

                // 4. Catalyst Chamber (Hanging Down)
                // Attached to bottom of valve block
                translate([0, 0, -15]) { // Bottom of block
                    rotate([0, 180, 0]) { // Pointing down

                        // Main Housing
                        color("DarkSlateGray")
                        difference() {
                            cylinder(h=cat_height, d=cat_diam);
                            translate([0,0,-1]) cylinder(h=cat_height+2, d=cat_diam-10);
                        }

                        // Catalyst Pellets
                        color("Black")
                        translate([0,0,10]) cylinder(h=cat_height-20, d=cat_diam-15);

                        // Cartridge Heater
                        color("Gold")
                        cylinder(h=cat_height + 20, d=8);

                        // Text Label (Adjusted for rotation)
                        color("White")
                        translate([0, -cat_diam/2 - 10, cat_height/2])
                        rotate([90,0,180]) // Rotated to be readable
                        text("Magnetite Cat.", size=8, halign="center");
                    }
                }
            }
        }
    }
}

// --- ASSEMBLY ---
difference() {
    union() {
        pressure_vessel();
        internal_coil_liner();
        smart_lid_assembly();
        side_plumbing_assembly();
    }
    // Cutaway to see inside
    translate([0, -200, -10]) cube([200, 200, 400]);
}
