$fn = 360 ;
margin = 0.01;
beam_height = 8.5;
redBeam_height = 15;

module smd0805(x, y, a=0) {
    smd0805_l = 3.7;
    smd0805_w = 1.9;
    smd0805_h = 1.0;
    translate([x, y, 0]) rotate([0, 0, a]) translate([-smd0805_l/2, -smd0805_w/2, -margin]) cube([smd0805_l, smd0805_w, smd0805_h+margin]);
}

module sideViewer(x, y, a=0) {
    sideViewer_l = 5.12;
    sideViewer_w = 2.30;
    sideViewer_h = 5.72;
    lense_d = 1.22;
    lense_dh = 1.22;
    
    beam_length = 10;
    beam_d2 = 2 * beam_length * tan(30);
    
    sideViewer_pitch = 2.54;
    sideViewer_legDim = 0.8;
    sideViewer_legLength = 12.70;
    
    translate([x, y, 0]) rotate([0, 0, a]) translate([0, 0, -(sideViewer_h-lense_dh-lense_d/2)+beam_height]) translate([-sideViewer_legDim/2, -sideViewer_legDim/2, 0]) {
        translate([0, 0, -sideViewer_legLength]) {
            translate([0, 0, 0])                cube([sideViewer_legDim, sideViewer_legDim, sideViewer_legLength+margin]);  // pin 1
            translate([sideViewer_pitch, 0, 0]) cube([sideViewer_legDim, sideViewer_legDim, sideViewer_legLength+margin]);  // pin 2
        };
        translate([-sideViewer_l/2+sideViewer_pitch/2+sideViewer_legDim/2, -sideViewer_w/2+sideViewer_legDim/2, 0]) cube([sideViewer_l, sideViewer_w+beam_length, sideViewer_h+10]);  // head
        translate([sideViewer_pitch/2+sideViewer_legDim/2, sideViewer_w/2+sideViewer_legDim/2, sideViewer_h-lense_dh-lense_d/2]) rotate([-90, 0, 0]) cylinder(h=beam_length, d1=lense_d, d2=beam_d2); // beam
    }
}

module bendT1(x, y, bentHeight=0, bentLength=4) {
    legDim = 0.8;
    legLen = 27;
    legPitch = 2.45;
    
    hLegLen = bentLength < 0 ? legLen-bentHeight : bentLength;
    vLegLen = legLen - hLegLen;
    
    translate([x-legPitch/2, y-hLegLen, bentHeight]) {
        translate([-4.0/2, -(3.0+4.0+hLegLen)/2, -4.0/2]) cube([4.0, 3.0+4.0+hLegLen, 4.0]);
        rotate([90, 0, 0]) {
            translate([-legDim/2+legPitch/2, -legDim/2, -hLegLen]) cube([legDim, legDim, hLegLen]);
            translate([-legDim/2-legPitch/2, -legDim/2, -hLegLen]) cube([legDim, legDim, hLegLen]);
        }
        translate([-legDim/2+legPitch/2, -legDim/2+hLegLen, -vLegLen]) cube([legDim, legDim, vLegLen]);
        translate([-legDim/2-legPitch/2, -legDim/2+hLegLen, -vLegLen]) cube([legDim, legDim, vLegLen]);
        
        rotate([90, 0, 0]) {
            cylinder(h=4.45-3.0/2, d1=3.1, d2=3.0);
            translate([0, 0, 4.45-3.0/2]) sphere(d=3.0);
            cylinder(h=1, d=3.2);
        }
    }
}

module m2(x, y, h=10) {
    m2_diameter = 2.2;
    m2_diameter_head = 6;
    translate([x, y, -h/4]) cylinder(h=h, r=m2_diameter/2);
    translate([x, y, h/4]) cylinder(h=100, r=m2_diameter_head/2);
}

module cuveHole(x, y, l=100, h=100, window_dh=0, a=0) {
    cuve_dim = 12.8; // with tolerance (12.5+/-0.2 and 0.1 extra margin)
    window_width = 6;
    window_height = 6;
    translate([x, y, 0]) {
        translate([-cuve_dim/2, -cuve_dim/2, -margin]) cube([cuve_dim, cuve_dim, window_dh+h+margin]);
        //translate([-window_width/2, -l/2, window_dh-margin]) cube([window_width, l, h+margin]);
        translate([-cuve_dim/2, -cuve_dim/2, 0]) cube([cuve_dim, cuve_dim, 45]);
    }
}

difference() {
    cube([22.86, 33.02, 15]);
    translate([-margin, -margin, -margin]) cube([13.335+margin, 2.54+margin, 3+margin]);
    
    sideViewer( 8.89,  5.08);  // Q1
    sideViewer(11.43, 25.40, a=180);  // D1
    

    smd0805( 8.89, 34.02, a=90);  // R3
    smd0805( 6.35, 34.02, a=90);  // R2
    smd0805(10.43,  2.54);  // R1

    m2(19.05,  4.445);
    m2( 3.81, 27.94);
    
    cuveHole(10.16, 15.24, l=25, h=15);
    bendT1(11.43, 30.48, redBeam_height);
}
