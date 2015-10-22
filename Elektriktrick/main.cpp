//
//  main.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 9/29/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "main.h"

#include "FL/Fl.H"
#include "FL/Fl_Window.H"

#include "ETSerialPort.h"
#include "ETOpenGLWidget.h"

#include "ETGMesh.h"

//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Polyhedron_3.h>
//
//typedef CGAL::Simple_cartesian<double>     Kernel;
//typedef CGAL::Polyhedron_3<Kernel>         Polyhedron;
//typedef Polyhedron::Halfedge_handle        Halfedge_handle;

/*
 
 +-----------------                         --------------+
 | ooo  Title                                             |
 +-----------------                         --------------+
 | Toolbox (click to apply, drag into workflow to add)    |
 +-----------+-----                         --+-----------+
 | machine   | 3D View                        | info for  |
 |  v wFlow1 |                                | selected  |
 |    action1|                                | tool      |
 |    action2|                                |           |
 |  > wFlow2 |
 |  > wFlow3 |
 | machine2  |
 |  ...      |
 :           :
 :           :
 +-----------+-----
 | status bar, connections, progress, etc.
 +-----------------
 
 */

/*
 Unrelated Sintratec files: 0a 0d

 /config/system/sysConfig.xml
 /prints/print.xml (probably should have been /config/printing/print.xml)
 there is also a log file

 Unrelated Sintratec commands:

 MSG1: Every 1.2 seconds: 57.6494 - 58.8565
 <- laser speed: 0 mm/s \r\nvac freq: 60 \r\nir sens freq: 5 \r\n

 MSG2: Every 1.2 seconds: 57.6494 (no spaces, no newline)
 <- <signal>
 <temperatures>
 <tempController id="5" name="Powder Surface" target="0">15</tempController>
 <tempController id="6" name="Heating Coil" target="0">15</tempController>
 <tempSensor id="0" name="IR Internal">21</tempSensor>
 </temperatures>
 </signal>
 <tempSensor id="1" name="Test Thermistor(p1)">11</tempSensor>
 </temperatures> (sic.)
 </signal>
 <- <signal>
 <status>
 <statusPrinting isPrinting="false">
 </status>
 </signal>


 First connection:
 -> <identify/>
 <- <printerID commVersion="1" model="KitCDC" id="512031204630374d3130313234303033"/>

 Move left piston up 0.1mm
 -> <axis id="1" home="false" type="relative" safe="true">0.1</axis>
 send 5 times, then:
 <- calling axis (id=1)\n

 Right piston ID is 2

 Apply powder:
 -> <applyPowder z="0.15"/>
 While applying powder, MSG1 continues, MSG2 is suspended
 When done, MSG2 resumes

 Move Sled:
 -> <applyLayer/>
 Messages as above, no confirmation

 Set Laser Speed:
 -> <setSpeeds><speed id="0" auto="false">100</speed></setSpeeds>
 <- setSpeeds active
 <- target_speed=100000 of axis=0\r\n
 <- setSpeeds inactive

 Setting temperatures (id5=lamps, id6=coil, set to 0.0 deg C to switch off):
 -> <setTemperatures>
 <tempController id="5">60.0</tempController>
 </setTemperatures>
 no confirmation

 Start the printing process (file is on the SD card)
 -> <startPrint name="print"/>
 <- <statusPrinting isPrinting="true" printName=" 0:prints/print.xml"/>
 then after a few seconds:
 <- <signal>
 <progress currentLayer="1" estimatedPercentage="0" expiredPrintTime="15" estimatedTimeToCompletion="15593" />
 </signal>

 Pause the printing:
 -> <pausePrint/>

 Stop printing:
 -> <abortPrint>
 (messes up the printing filename, but does not stop the heaters, likely stops the print as a bug)
 (restarting does not do anything useful)

 Start test pattern (can't be stopped AFAIK)
 -> <testPattern id="1"/>

 Print file format (xml using tabs):
 <?xml version='1.0' encoding='UTF-8'?>
 <!--Instructions for KitCDC-->
 <instruction>
 <print>
 <layer z="0.200">               # machine assumes that layer 0.0 is spread, then spreads 0.1, heats, spreads 0.2
 <j x="4.950" y="4.950"/>    # -4.95 to 4.95 = 10mm taking laser radius (0.05) into account
 <g x="-4.950" y="4.950"/>
 <g x="-4.950" y="-4.950"/>
 ...
 <g x="-4.900" y="-4.010"/>
 <j x="-4.900" y="-3.797"/>
 <g x="-3.797" y="-4.900"/>
 <j x="-3.585" y="-4.900"/>
 ...
 <j x="4.646" y="4.900"/>
 <g x="4.900" y="4.646"/>
 </layer>
 <layer z="0.300">
 <j x="4.950" y="4.950"/>
 <g x="-4.950" y="4.950"/>
 ...
 <g x="-4.646" y="4.900"/>   # layer 10.100 is the last layer
	</layer>
	<layer z="10.200">
 </layer>
 </print>
 </instruction>

 x1
 x2
 y1
 y2
 time
 scan
 init
 print
 units
 space
 layer
 galvo
 motor
 laser
 speeds
 signal
 status
 boxSls
 sensor
 message
 goChain
 printer
 surface
 command
 feedSls
 goFromTo
 identify
 bind_pid
 bangbang
 bottomSls
 go_config
 setSpeeds
 bind_galvo
 bind_laser
 bind_motor
 feedbedSls
 abortPrint
 pausePrint
 startPrint
 galvo_axis
 motor_axis
 laser_axis
 applyLayer
 printbedSls
 instruction
 temperature
 resumePrint
 bind_sensor
 applyPowder
 testPattern
 temperatures
 systemConfig
 power_driver
 statusMessage
 requestStatus
 bind_bangbang
 statusPrinting
 displayMessage
 tempController
 printParameters
 requestProgress
 setTemperatures
 temperature_axis
 bind_power_driver
 print_style_config
 requestTemperatures

 */



int main (int argc, char **argv)
{
//    Polyhedron P;
//    Halfedge_handle h = P.make_tetrahedron();

    Fl::args(argc, argv);
    Fl_Window *win = new Fl_Window(800, 500, "Elektriktrick");
    ETOpenGLWidget *ogl = new ETOpenGLWidget(10, 10, 780, 470);
    ogl->show();
    ETSerialPort *ser = new ETSerialPort(740, 485, 50, 10);
    ser->open("/dev/tty.usb1411", 19200);
    win->resizable(ogl);
    win->show();

//    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/ETCalibrate_v2_x02_y01.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/yoda-figure.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/trunicos40mm.stl");
//    loadStl("/Users/matt/female.stl");
    loadStl("/Users/matt/Desktop/Machine Shop/Project InMoov/WeVolver/Bicep_for_Robot_InMoov/SpacerV1.stl");

    Fl::run();
    return 0;
}
