# About the gSwitch18 Project #

A friend and I wanted a compact USB game controller interface to provide at least 10 switch inputs -- primarily for use with MS Flight Sim X.

We had experience with Atmel AVR chips and development tools (OS X and Ubuntu Linux) and research soon found us Objective Development's [V-USB project](http://www.obdev.at/products/vusb/index.html).

So we started with a [KiCAD](http://www.kicad-pcb.org) design ...

![http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0-KiCAD.jpg](http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0-KiCAD.jpg)

... and ended up with a working v1.0 prototype ...

![http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0.jpg](http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0.jpg)

Below we see a screen shot from Windows 7's Game Controller Properties, showing the device in use, with switch 17 in the 'on' state ...


![http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0-Windows.jpg](http://gswitch18.googlecode.com/svn/wiki/About.attach/prototype_v1-0-Windows.jpg)

# Details #

Our device uses an ATmega88p clocked at 12MHz, implementing a V-USB powered (bit-bang) USB HID compliant game controller interface.

Since this is an HID compliant 'Game Controller' device, no drivers are required for the host system, which is nice.

We use KiCAD for as our PCB design (EDA suite) because it is completely free (as in 'speech' _and_ 'beer') and we had used it very successfully before, for another [larger project](http://gruvin9x.googlecode.com).

We get production quality "prototype" PCB's made in China, using [ITead Studio](http://imall.iteadstudio.com/)'s [Open PCB prototyping service](http://imall.iteadstudio.com/open-pcb/pcb-prototyping.html). These are high quality, fully clear routed, masked and silk-screened, all at a fraction the price of bare copper, no mask, no silk-screen prototype boards from, 'the West' and yet also just a fraction the price!

# Make Your Own #
Please see the Google code 'source' link (above) to gain access to the completely open KiCAD design files and firmware source code.

# Buy Ready Made #
Hand built boards are now available at the [gruvin9X online store](http://gruvin9x.com/shop), in the 'gizmo' section.
