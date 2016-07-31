# obs-controller

I built a hardware controller for Open Broadcaster Software

The controller has broadcast buttons which have two LEDs inside in my case green and red

If your activate both LEDs the color turns yellow

I use 4 shiftregisters of the type SN74HC595N to controll the leds with only 3 digital pins of the arduino

The other pins are used to detect kepresses

If a button is pressed the arduino sends a keycommand to the computer

The keycombinations are set to trigger scenes or transitions in obs and to start/stop the stream and recording

The LEDs reflect which scene is currently live and which is on preview

Theres no APi in OBS so changes that are made in obs without the controller are not visible on the controller


Feel free to use the code and contact me if you have any questions (english/german)

Also check out the company that inspired this build -> SKAARHOJ http://skaarhoj.com
