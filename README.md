# RuuviTag nRF52 Bootloader & Example firmware projects
[![RuuviTag](https://tag.ruuvi.com/assets/images/fb_ruuvitag.jpg)](https://tag.ruuvitag.com)


## Repository structure
This repository is structured as follows:

```
.
+-- bootloader
|   +-- ruuvitag\_HW\_FLAVOR
|   |   +-- armgcc
|   |   |   +-- Makefile
|   |   |   +-- Linkerscript
|   |   +-- config
|   |   |   +-- sdk_configuration
|   |   +-- bootloader_files
+-- bsp
|   +-- BSP files
+-- drivers
|   +-- bme280
|   +-- lis2dh12
|   +-- etc
|   +-- init
|   |   +-- common_configuration
+-- libraries
|   +-- acceleration
|   +-- base64
|   +-- base91
+-- keys
|   +-- ruuvi_open_private.pem
+-- ruuvi_examples
|   +-- APPLICATION
|   |   +-- application_sdk_configuration
|   |   +-- application_bsp_configuration
|   |   +-- application_bluetooth_configuration
|   |   +-- ble_services
|   |   +-- ruuvitag_HW
|   |   |   +-- s132
|   |   |   |   +-- armgcc
|   |   |   |   |   +-- Makefile
|   |   |   |   |   +-- Linkerscript
|   |   |   |   +-- config
|   |   |   |   |   +-- board_sdk_configuration
|   |   |   |   |   +-- board_bsp_configuration
|   |   |   |   |   +-- board_bluetooth_configuration
|   |   +-- application files
+-- Makefile
+-- README.md
+-- .gitignore
|   +-- (SDK)
```

### Bootloader
The Bootloader folder contains DFU bootloader which is used to upload new software to your RuuviTag
without J-Link programmer, you can even use your smartphone and upload software over bluetooth.
Starting from SDK12 the bootloader uses secure, signed packages. The encryption keys used to validate these
software packages is split in two parts: dfu_public_key.c and your private key in "keys" folder.
More details on signing and keys are explained on DFU package creation section.

### BSP
BSP folder contains "Board Service Packages" which provide abstraction and portability between different boards. If you're interested in creating a custom board, create a custom board header file such as "ruuvitag\_b3.h" and add your board header file to "custom\_boards.h".

### Builds
Builds are in the Github [project releases](https://github.com/ruuvi/ruuvitag_fw/releases). The released packages are generally tested, but you should rely on [RuuviLab](https://lab.ruuvi.com/dfu/) if you're end-user rather
than developer.

### Drivers
Drivers folder contains the peripheral drivers such as a driver for SPI as well as drives for sensors on PCB. 

### Libraries
Libraries contain software routines which may not have hardware dependencies, i.e. they should run on your pc as well as on RuuviTag.

### Ruuvi Examples
Ruuvi examples has example firmware projects which can be used as a basis for your own application. 
The top-level folder of application contains application code and configuration, and there is a subfolder for
each hardware which can run the application. If the application requires softdevice,
create a folder with softdevice name "s132" to let the users know that a softdevice is required.
Configuration folder sets up board specific configuratuin, such as pins.
Armgcc folder contains makefile and linker script.

### SDK
The SDK folder contains Nordic Software development kit which is used to provide various 
low-level drivers and abstractions to speed up development. We do not host the SDK to reduce the 
size of repository, our makefile downloads and unzips the SDK if it is not present. 

### Licenses
Please note that these examples inherit a lot of code from various sources and pay careful attention to 
license and origin of each application. Most importantly, the code will be statically linked against
Nordic Softdevice, for which the source code is not available. Therefore the code is not GPL-compatible.

## Developing Ruuvi Firmware

Instructions below are tested using OS X and Ubuntu, but basically any Unix distribution (or even Windows) should be fine. Compilation works also using the *Bash on Ubuntu on Windows* -feature added in the July 2016 update of Windows 10 if you follow the Ubuntu directions. If you've compiled and flashed successfully (or unsuccessfully), please identify yourself on our Slack :)

We also host some ready binaries so it's not necessary to setup a development environment if you would be happy to use those. So, please check a `builds` directory first. If you would like to modify the firmware code, continue reading:

### Prerequisites (to compile):

The project currently uses the Nordic nRF52 SDK version 12.3.0 (downloaded in the `make` process)
and thus requires the GNU ARM Embedded Toolchain version 4.9 Q3 2015 (aka 4.9.3) for compiling:

* Download and install [GNU ARM Embedded Toolchain 4.9](https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update)

For example on Xubuntu 16.04.3 using:

```
cd ~/Downloads/
wget https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
sudo tar xvfj gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2 -C /usr/local
sudo apt-get install -y lib32ncurses5 lib32z1
echo 'PATH="/usr/local/gcc-arm-none-eabi-4_9-2015q3/bin:$PATH"' >> $HOME/.profile
source $HOME/.profile
# check version, expecting: 4.9.3 20150529 (release)
arm-none-eabi-gcc --version
```

Changing the PATH might not be needed as the toolchain will use the path defined in the SDK Makefile.
Adjust the GNU_INSTALL_ROOT inside your Makefile in the `$SDK/components/toolchain/gcc/` folder
when using another destination than the `/usr/local` shown above.

Note that the nRF52 SDK will be downloaded in `make`, so only after this will
the `$SDK/components/toolchain/gcc/` folder exist in the project (typically
as the `nRF5_SDK_12.3.0_d7731ad/components/toolchain/gcc/` folder).

### Prerequisites (to create DFU distribution .zip packages)

Instructions how to install (on OS X, Ubuntu):

Install pip:

```
curl -O https://bootstrap.pypa.io/get-pip.py
sudo python get-pip.py
```

(Option 1) Install latest nrfutil from pip:

```
sudo pip install nrfutil
# check version, expecting: 3.4.0 (or newer)
nrfutil version
```

(Option 2) Install nrfutil from source:


```
git clone https://github.com/NordicSemiconductor/pc-nrfutil.git
cd pc-nrfutil
sudo pip install -r requirements.txt
sudo python setup.py install
# check version, expecting: 3.4.0 (or newer)
nrfutil version
```

To get started you can try:

```
nrfutil pkg generate --debug-mode --application app.hex --key-file key.pem app_dfu_package.zip
```

Debug mode skips various version checks which is useful for development. Packages have to be signed,
RuuviTag ship with bootloader that accepts packages signed with the `keys/ruuvi_open_private.pem` key.

More examples and details can be found at [nrfutil repository](https://github.com/NordicSemiconductor/pc-nrfutil).

# Compiling

`make` downloads Nordic Semiconductor's nRF52 SDK and extracts it. First time use will probably fail as SDK Makefile defines the path of toolchain, and the default path might differ from your system. 

Modify $SDK/components/toolchain/gcc/Makefile.posix (on Linux and OSX) or Makefile.windows on windows
to point to your gcc-arm install location. 

You need also add support for secure bootloader elliptic curve cryptography by installing micro-ECC inside
SDK, details can be found at [Nordic Infocenter] (https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Flib_crypto.html)

Second time running `make` builds all the sources. 
`make clean` cleans the build directories.

For more help, please join [Ruuvi Slack](http://slack.ruuvi.com).

# Flashing

## With Segger J-Link
If the device is empty (no SoftDevice S132 + bootloader flashed), you need to flash using SWD interface. The easiest way is to use nRF52 development kit (PCA10036 or PCA10040) with embedded Segger. Steps:

Download and install latest [J-Link](https://www.segger.com/jlink-software.html)

Start the J-Link from command line by typing:

`JLinkExe -device nrf52 -if swd -speed 1000`

SoftDevice is Nordic Semiconductor's Bluetooth Smart (or ANT) protocol stack. Sources are super secret, but the latest version is always bundled with the SDK. So, let's flash it:

`J-Link>loadfile nRF5_SDK_11.0.0_89a8197/components/softdevice/s132/hex/s132_nrf52_2.0.0_softdevice.hex`

After the SoftDevice is flashed successfully, flash the bootloader:

`J-Link>loadfile bootloader/ruuvitag_b2/dual_bank_ble_s132/armgcc/_build/ruuvitag_b2_bootloader.hex`

## With nrfjprog
Get nrfjprog from Nordic's website: TODO LINK
nrfjprog offers simple wrapper to Segger's JLINK. To get started, erase your device:
`nrfjprog --family nrf52 --eraseall`
Then flash softdevice + bootloader:
`nrfjprog --family nrf52 --program latest_softdevice.hex`
`nrfjprog --family nrf52 --program latest_bootloader.hex`
Once you're ready, reset the device and verify yhe bootloader is broadcasting with your smartphone:
`nrfjprog --family nrf52 -r`

## Over the Air
Once softdevice and bootloader are installed no cables are needed, ever (unless the device needs to be rescued for some reason)! From now on, the FW (and/or the bootloader and/or the SoftDevice) can be updated Over-The-Air using Nordic's nRF Toolbox (or MasterControl Panel):

https://www.nordicsemi.com/eng/Products/nRFready-Demo-Apps/nRF-Toolbox-App

https://github.com/NordicSemiconductor/Android-nRF-Toolbox

https://github.com/NordicSemiconductor/IOS-nRF-Toolbox

# If you're a developer, this is probably what you're after:

1. Flash the SoftDevice protocol stack
2. Compile the bootloader (and the application)
3. Flash the bootloader
4. Reset the device
5. Create .zip distribution package (that includes at least the application code)
6. Install nRF Toolbox (Android/iOS)
7. Press DFU OTA button
8. After completed, press it again! And again! No more cables needed ^^
9. Now you can update SoftDevice and/or bootloader and/or application using DFU OTA. Cool, huh?
