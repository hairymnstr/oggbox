module switch() {
	union() {
		translate([0,0.9,0]) cube([6.6,1.8,6.2],center=true);
		translate([0,2.3,0]) rotate([90,0,0]) cylinder(r1=1.5, r2=1.5, h=1, $fn=16, center=true);
	}
}

module side_switch() {
	union() {
		translate([0,0.9,0]) cube([4,1.8,6],center=true);
		translate([0,0.9,3.75]) cube([2.4,1.6,1.5],center=true);
	}
}

module sdcard_holder() {
	difference() {
		translate([0,0,-13.25]) cube([25,2.5,26.5]);
		translate([-0.1,-0.1,-13.35]) cube([5.1,3,2.1]);
		translate([5,1.25,-13.25]) rotate([0,45,0]) cube([2.828,3,2.828],center=true);
	}
}

module jack_socket() {
	union() {
		translate([-3.5,0,2.5]) cube([7,6,12]);
		translate([0,3,1.5]) rotate() cylinder(r1=3, r2=3, h=2.5, $fn=16, center=true);
	}
}

module usb_socket() {
	translate([-3.75,0,0]) cube([7.5,3,5.5]);
}

module programming_header() {
	translate([-7.62,0,-1.27]) cube([15.24,8.5,2.54]);
}

module pcba() {
	union() {
		// PCB
		difference() {
			cube([50.0,1.6,100.0], center = true);
			translate([25,0,50]) rotate([0,45,0]) cube([3.5355,3.5355,3.5355], center=true);
			translate([-25,0,50]) rotate([0,45,0]) cube([3.5355,3.5355,3.5355], center=true);
			translate([25,0,-50]) rotate([0,45,0]) cube([10.6066,10.6066,10.6066], center=true);
			translate([-25,0,-50]) rotate([0,45,0]) cube([10.6066,10.6066,10.6066], center=true);
		}
		// screen
		translate([25-42.2,0.8,9.5-27.3]) cube([42.2,4.405,54.6]);
		// buttons
		translate([15,0.8,44]) switch();
		translate([-15,0.8,44]) switch();
		translate([0,0.8,-25]) switch();
		translate([0,0.8,-45]) switch();
		translate([10,0.8,-35]) switch();
		translate([-10,0.8,-35]) switch();

		// volume switches
		translate([-22,0.8,31.25]) rotate([0,270,0]) side_switch();
		translate([-22,0.8,23]) rotate([0,270,0]) side_switch();
		// reset switch
		translate([-14.75,-0.8,-45]) rotate([0,180,180]) side_switch();

		// hold switch

		// sd card holder
		translate([3.75,-0.8,-26.5]) rotate([180,180,0]) sdcard_holder();

		// jack sockets
		translate([-17,-0.8,52.25]) rotate([0,180,180]) jack_socket();
		translate([17,-0.8,52.25]) rotate([0,180,180]) jack_socket();

		// USB socket
		translate([0,-0.8,-51]) rotate([180,180,0]) usb_socket();

		// programming header
		translate([22.75,-0.8,-27]) rotate([180,90,0]) programming_header();

		// Cortex-debug connector

		// battery connector

	}
}

pcba();