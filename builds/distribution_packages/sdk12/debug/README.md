#Ruuvitag example programs

These programs showcase RuuviTags capablities. This is very much a continous work-in progress.

If you have ideas on which kind of demo application you'd like to see, drop us a note in our [trello board](https://trello.com/b/kz1llpvK/ruuvitag-firmware). 

Ble\_app\_beacon is a simple beacon application which broadcasts fixed UUID and blinks led to display activity.
It's mainly used to test BLE-functions and DFU bootloaders. 

Weather Station is the example shipped in the Kickstarter devices.
_Measured current consumption: 35µA on average in main loop, 4 µA in idle andd 9 mA peak._

Debug-bootloader is the debug version of our bootloader which skips version checks in DFU packages.
It's recommended to use this bootloader while developing your applications. The bootloader accept
packages signed with [Ruuvis open private key published here.](../../../keys/ruuvi-open-private.pem)

Template_application initializes sensors and waits for a minute user to press button. 
If user presses button during that minute, application enters main loop, otherwise 
the application will shut down to preserve battery.
_Measured current consumption: 4µA on average in main loop, 1.3 µA in shutdown._


