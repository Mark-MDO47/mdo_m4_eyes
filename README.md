# mdo_m4_eyes
"fork" of https://github.com/adafruit/Adafruit_Learning_System_Guides.git /M4_Eyes directory

This Adafruit code is incredible! I just want to make some tiny customizations for personal use.

I didn't want to fork the entire Adafruit_Learning_System_Guides so I am making a copy of the .../M4_Eyes directory and all subdirectories.

I am including the LICENSE from Adafruit_Learning_System_Guides - an MIT license.
- I am not trying to pretend to be approved by Adafruit in any way; I just want to ensure that all of the Adafruit license terms are respected in this "fork".

**Table Of Contents**
* [Top](#mdo_m4_eyes "Top")
* [Directory Structure](#directory-structure "Directory Structure")
* [Skull Project](#skull-project "Skull Project")
* [Switch Eye Config Each Reset](#switch-eye-config-each-reset "Switch Eye Config Each Reset")
  * [Curiously](#curiously "Curiously")

## Directory Structure
[Top](#mdo_m4_eyes "Top")<br>
The directory **M4_Eyes/\*** files for this repo originally came from https://github.com/adafruit/Adafruit_Learning_System_Guides.git SHA1 ID 9de211fb39df0d7ae9fd7d7cd6783d744092a764 committed 2023-11-29 13:28:01.
- This corresponds in this repo to directory **M4_Eyes/** SHA1 ID 5e363245c773873216ca4be0e4efd50aad399080 committed 2023-12-03 13:18:35

Other directories are for my projects.

## Skull Project
[Top](#mdo_m4_eyes "Top")<br>
This code for the Adafruit HalloWing M4 Express https://www.adafruit.com/product/4300 is in directory **mdo_m4_skull_project**.

This is the code for https://github.com/Mark-MDO47/Skull-Project.

My addition to the code is in file **mdo_skull_project.cpp**. **mdo_m4_skull_project.ino is** just a copy of **M4_Eyes.ino** renamed so I can use it from this directory.

The functionality is to use the PIR sensor https://www.adafruit.com/product/189 connected to the HalloWing sensor port to control backlight on/off. An LED push button switch is used to determine whether the backlight should be ALWAYS-ON or if it should come on based on the PIR sensor.

## Switch Eye Config Each Reset
[Top](#mdo_m4_eyes "Top")<br>
This code for the Adafruit HalloWing M4 Express https://www.adafruit.com/product/4300 is in directory **mdo_m4_eyes**.

I will switch among the various eye configuration files. I start with the list in the "eyes" directory (which will be copied to the root  directory for the board) but omit the hazel_128x128 since this repo is for the Hallowing M4 not the Hallowing M0.

The approach will be file based. I considered using EEPROM but the SAMD5 M4 uses a "SmartEEPROM" which is emulated from a space in the normal FLASH memory and not on a special EEPROM area. Thus using EEPROM with the M4 is approximately equivalent in terms of writes to FLASH to using the file system. FLASH is normally specified as about 10,000 writes and that is the case here, see Table 54-41 Flash Endurance and Data Retention.
- https://www.mouser.com/datasheet/2/268/SAM_D5x_E5x_Family_Data_Sheet_DS60001507-3107027.pdf

So if using this code, do not reboot more than 10,000 times.

If the file **mdo_m4_eyes.txt** is present in the root directory of the board, we will switch each time we reset. If mdo_m4_eyes.txt is not present, we will use the **config.eye** file in the root directory as was normally done.

mdo_m4_eyes.txt will be a list of config files to cycle through separated by lf or crlf; below is a small example
```
hazel/config.eye
anime/config.eye
demon/config.eye
```

If the code reads the list above, it would boot using hazel/config.eye and re-write mdo_m4_eyes.txt to put the current configuration at the end as follows:
```
anime/config.eye
demon/config.eye
hazel/config.eye
```

This allows me to put the cycle in any order and to have some eyes show up more often than others or not appear at all.

### Curiously
[Top](#mdo_m4_eyes "Top")<br>
It appears as if booting without the USB plugged into a PC means that the psuedo-drive containing the files doesn't work properly at first. I am getting fails from arcada.exists() with no USB plugged in and success when the USB is plugged in. Not sure what this means - lots of code in those file access libraries to peruse.

Because of this I may back up and use EEPROM after all.
