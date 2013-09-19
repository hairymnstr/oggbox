module screen() {
	union() {
		difference() {
			translate([1.3,1,2.95]) cylinder(r1=5,r2=5,h=14.7,$fn=16, center=true);
			translate([-5,4.405,-5]) cube([10,5,20]);
		}
		translate([-1,0,-27.3]) cube([42.2,4.405,54.6]);
		translate([9.25,-2.75,2.95]) cube([17.5,2.5,14.7], center=true);
	}
}

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

module hold_switch() {
	union() {
		translate([-3.35,0,-1.4]) cube([6.7,1.95,2.8]);
		translate([-1.5,.575,1.4]) cube([1.35,.8,1.9]);
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

module cortex_debug() {
	translate([0,2.7,0]) cube([13.65,5.4,5.6], center=true);
}

module battery_connector() {
	translate([-5,0,-9.5]) cube([10,6,11.5]);
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
		translate([24,0.8,9.5]) rotate([0,180,0]) screen();

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
		translate([-23.5,0.8,-9]) rotate([0,270,0]) hold_switch();

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
		translate([-20,-0.8,27.25]) rotate([180,90,0]) cortex_debug();

		// battery connector
		translate([16,-0.8,-42.75]) rotate([180,0,0]) battery_connector();

	}
}

module sdcard() {
	difference() {
		cube([24,2.1,32], center=true);
		translate([-12,0,-16]) rotate([0,45,0]) cube([5.65,5.65,5.65], center=true);
	}
}

module button_hole() {
	union() {
		translate([0,4.5,0]) rotate([90,0,0]) cylinder(r1=3,r2=3,h=4,$fn=32,center=true);
		rotate([270,0,0]) cylinder(r1=5,r2=5,h=9.6,$fn=32,center=true);
	}
}

module side_switch_hole() {
	union() {
		translate([2.5,0,0]) cube([4,5,10],center=true);
		cube([3,3,3],center=true);
		translate([0,0,-1.5]) rotate([0,90,0]) cylinder(r1=1.5,r2=1.5,h=3,$fn=32,center=true);
		translate([0,0,1.5]) rotate([0,90,0]) cylinder(r1=1.5,r2=1.5,h=3,$fn=32,center=true);
	}
}

module hold_switch_hole() {
	union() {
		translate([2.5,0,0]) cube([4,5,10],center=true);
		cube([3,3,4.5],center=true);
		translate([0,0,-2.25]) rotate([0,90,0]) cylinder(r1=1.5,r2=1.5,h=3,$fn=32,center=true);
		translate([0,0,2.25]) rotate([0,90,0]) cylinder(r1=1.5,r2=1.5,h=3,$fn=32,center=true);
	}
}

module upper_case() {
	union() {
	difference() {
		union() {
		difference() {
			translate([-27,-3,-49.5]) {
				minkowski() {
					difference() {
						cube([54,6,99]);
						rotate([0,45,0]) cube([sqrt(50),20,sqrt(50)],center=true);
					}
					sphere(r=3,$fn=32);
				}
			}
			translate([0,-3,0]) cube([62,6,106],center=true);
		}
		translate([-17,0,51.5]) cube([6.5,6.5,2],center=true);
		translate([17,0,51.5]) cube([6.5,6.5,2],center=true);
		
		}

		// button holes
		translate([15,0,44]) button_hole();
		translate([-15,0,44]) button_hole();
		translate([0,0,-25]) button_hole();
		translate([0,0,-45]) button_hole();
		translate([10,0,-35]) button_hole();
		translate([-10,0,-35]) button_hole();

		// volume switch holes
		translate([-29,1.6,31.25]) side_switch_hole();
		translate([-29,1.6,23]) side_switch_hole();
		translate([-29,0,31.25]) cube([3,3,10],center=true);
		translate([-29,0,23]) cube([3,3,10],center=true);

		// hold switch hole
		translate([-29,1.6,-9]) hold_switch_hole();
		translate([-29,0,-9]) cube([3,3,10],center=true);
		
		translate([-17,-0.8,52.5]) rotate([0,0,180]) jack_hole();
		translate([17,-0.8,52.5]) rotate([0,0,180]) jack_hole();

		// pcb hole
		difference() {
			cube([51,5.6,101],center=true);
			translate([25.5,0,50.5]) rotate([0,45,0]) cube([3.5355,10,3.5355],center=true);
			translate([-25.5,0,50.5]) rotate([0,45,0]) cube([3.5355,10,3.5355],center=true);
			translate([-25.5,0,-50.5]) rotate([0,45,0]) cube([10.6066,10,10.6066],center=true);
			translate([25.5,0,-50.5]) rotate([0,45,0]) cube([10.6066,10,10.6066],center=true);
		}

		// screen recess
		translate([24,0.8,9.5]) translate([-43.5,-2,-28]) cube([45,6.5,56]);
		
		// screen hole
		translate([-15.5,4,-25.75+9.5]) cube([31,5,51.5]);

		// screen ribbon clearance
		translate([24,0.8,9.5]) translate([-1.3,-1,-2.95]) cylinder(r1=5,r2=5,h=16,$fn=16,center=true);

		// usb socket
		translate([0,0,-51]) usb_socket_hole();
	}
	// PCB support bars
	translate([-1,0.8,41]) cube([2,3,10]);
	translate([-24.5,0.8,-46]) cube([15,3,2]);
	translate([9.5,0.8,-46]) cube([15,3,2]);
	translate([21,0.8,38]) cube([5,3,2]);
	translate([-26,0.8,38]) cube([5,3,2]);
	}

}

module jack_hole() {
	union() {
		translate([0,3.25,-9]) cube([8,7,13],center=true);
		translate([0,3,-2.6]) rotate() cylinder(r1=3.25,r2=3.25,h=4,$fn=32);
	}
}

module usb_socket_hole() {
	union() {
		translate([-4,-3.3,-1]) cube([8,3.5,5]);
		translate([-6.5,-5.8,-5]) cube([13,8,5]);
	}
}

module lower_case() {
union() {
	difference() {
	union() {
		translate([-29.25,0.8,31.25]) cube([1.5,1.6,10],center=true);
		translate([-29.25,0.8,23]) cube([1.5,1.6,10],center=true);
		translate([-29.25,0.8,-9]) cube([1.5,1.6,10],center=true);
		difference() {
			translate([-27,-9,-49.5]) {
				minkowski() {
					difference() {
						cube([54,12,99]);
						rotate([0,45,0]) cube([sqrt(50),30,sqrt(50)],center=true);
					}
					sphere(r=3,$fn=32);
				}
			}
			translate([0,3,0]) cube([62,6,106],center=true);
		}
		}

		// volume switch holes
		translate([-29,1.6,31.25]) side_switch_hole();
		translate([-29,1.6,23]) side_switch_hole();

		// hold switch hole
		translate([-29,1.6,-9]) hold_switch_hole();

		// pcb hole
		difference() {
			cube([51,7.6,101],center=true);
			translate([25.5,0,50.5]) rotate([0,45,0]) cube([3.5355,10,3.5355],center=true);
			translate([-25.5,0,50.5]) rotate([0,45,0]) cube([3.5355,10,3.5355],center=true);
			translate([-25.5,0,-50.5]) rotate([0,45,0]) cube([10.6066,10,10.6066],center=true);
			translate([25.5,0,-50.5]) rotate([0,45,0]) cube([10.6066,10,10.6066],center=true);
		}

		// screen ribbon clearance
		translate([24,0.8,9.5]) translate([-1.3,-1,-2.95]) cylinder(r1=5,r2=5,h=16,$fn=16,center=true);

		// jack holes
		translate([-17,-0.8,52.5]) rotate([0,0,180]) jack_hole();
		translate([17,-0.8,52.5]) rotate([0,0,180]) jack_hole();
		translate([-17,0,52.5]) cube([6.5,6.5,6],center=true);
		translate([17,0,52.5]) cube([6.5,6.5,6],center=true);
		// cortex debug connector
		translate([-22.5,-6.8,19.25]) cube([7,6,16]);

		// programming header
		translate([22.75-(3.8/2),-9.3,-27 - (15.24/2)]) cube([3.8,8.5,15.24]);

		// sd card slot
		translate([-16.5-16,-2.205-1.25,-26.75-13]) cube([32,3.5,26]);

		// usb socket
		translate([0,0,-51]) usb_socket_hole();

		// battery cutout
		translate([-17,-10.3,-62+50-14.5]) cube([38,7.5,62]);
		translate([16,-0.8,-42.75]) rotate([180,0,0]) translate([-6,0,-18]) cube([12,7,20]);
		// and a bit of room for the wire
		translate([-13,-6.5,-43]) cube([25,3,20]);
	}
	translate([-25.5,-4.8,-13]) cube([8,4,2]);
	translate([21.5,-4.8,16]) cube([5,4,2]);
	translate([22.5,-4.8,-46]) cube([4,4,10]);
	translate([14,-4.8,-51]) cube([8,4,4]);
	translate([-8,-4.8,-51]) cube([3,4,3]);
	translate([-26,-4.8,-45.5]) cube([3,4,4.5]);
	translate([21,-4.8,-13]) cube([5,4,3]);
	translate([5,-4.8,45]) cube([3,4,5]);
	translate([-9,-4.8,45]) cube([3,4,5]);
	}
}


module assembly() {
	translate([-16.5,-2.205,-26.75]) rotate([0,270,0]) sdcard();

	pcba();
	upper_case();
	lower_case();
}

module printable() {

	translate([-35,0,6]) rotate([270,0,0]) upper_case();
	translate([35,0,12]) rotate([90,0,0]) lower_case();
}

printable();
//assembly();