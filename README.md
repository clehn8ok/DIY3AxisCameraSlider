# DIY3AxisCameraSlider
3D Printed 3 Axis Camera Slider, remote controlled via Bluetooth PS3 Controller
![thumbnail_NEW_FORMAT_camerasli_der](https://user-images.githubusercontent.com/86553872/123544929-ef2fa080-d755-11eb-9e9e-7710392a2718.jpg)

Build Video:https://youtu.be/7TkybpSQULk


This project is based on isaac879's slider. I incorporated the pan & tilt mechanism from him.
The brain is a esp32 uC with bluetooth, which is controlled via a generic PS3 Controller.
You can set keyframes via square button, and let the slider autonomously circle between the positions. You can also change the speed via L1&R1 to shoot timelapses.
total cost for the slider was about ~100€

Libraries used:

-FastAccelStepper

-PS3 Controller Host

don't h8 on me, for the coding, i am a noob :D


3D Printed Parts (in the folder STL):
PLA
use 30% infill 
gears, and small parts with 0.2mm Layerheight, everything else with 0.3mm layerheigt

Parts needed:

-3xNema17 Steppers -> https://amzn.to/3dlSE6m

-3xTMC2208 Stepper drivers -> https://amzn.to/3jiBDOd

-1xESP32 uC  -> https://amzn.to/2UJMW84

-1xDC-DC Buck Converter for the ESP32

-1x12V LiPo Battery

-3xA3144 Hall Effect Sensors

-3x10k Resistors

-3x100uF Capacitors

-2xF623zz ball bearings

-1xPS3 Controller -> https://amzn.to/3hdXDXW

-wiresset of connectors (JST 2.0 & 2.5 & dupont)

-6mm airsoft balls (for the ballbearing)

-2080 V-SLot Extrusion

-GT2 Belt

-screw set (M3,M4 & M5)

-4xVslot Wheels(22x16x10,2)

