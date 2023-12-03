# mdo_m4_eyes
"fork" of https://github.com/adafruit/Adafruit_Learning_System_Guides.git /M4_Eyes directory

I didn't want to fork the entire Adafruit_Learning_System_Guides so I am making a copy of the .../M4_Eyes directory and all subdirectories.

This Adafruit code is incredible! I just want to make some tiny customizations for personal use.

I am including the LICENSE from Adafruit_Learning_System_Guides - an MIT license.
- I am not trying to pretend to be approved by Adafruit in any way; I just want to ensure that all of the Adafruit license terms are respected in this "fork".

**Table Of Contents**
* [Top](#mdo_m4_eyes "Top")<br>
* [Directory Structure](#directory-structure "Directory Structure")
* [Changes](#changes "Changes")
  * [Switch Eye Config Each Reset](#switch-eye-config-each-reset "Switch Eye Config Each Reset")

## Directory Structure
[Top](#mdo_m4_eyes "Top")<br>
The directory **M4_Eyes/\*** files for this repo originally come from https://github.com/adafruit/Adafruit_Learning_System_Guides.git SHA1 ID 9de211fb39df0d7ae9fd7d7cd6783d744092a764 committed 2023-11-29 13:28:01.
- This corresponds in this repo to SHA1 ID 5e363245c773873216ca4be0e4efd50aad399080 committed 2023-12-03 13:18:35

My version of this code for the Adafruit HalloWing M4 Express https://www.adafruit.com/product/4300 is in directory **mdo_m4_eyes**.

## Changes
[Top](#mdo_m4_eyes "Top")<br>
| Description | Link |
| --- | --- |
| switch eye configuration each time I reset or power-cycle | [Switch Eye Config Each Reset](#switch-eye-config-each-reset "Switch Eye Config Each Reset") |

### Switch Eye Config Each Reset
[Top](#mdo_m4_eyes "Top")<br>
I will switch among the various eye configuration files. I start with the list in the "eyes" directory (which will be copied to the D:\ directory) but omit the hazel_128x128 since this is for the Hallowing M4 not the Hallowing M0.

The approach will be file based. I considered using EEPROM but the SAMD5 M4 uses a "SmartEEPROM" which is emulated from a space in the normal FLASH memory and not on a special EEPROM area. Thus using EEPROM with the M4 is approximately equivalent in terms of writes to FLASH to using the file system. FLASH is normally specified as about 10,000 writes and that is the case here, see Table 54-41 Flash Endurance and Data Retention.
- https://www.mouser.com/datasheet/2/268/SAM_D5x_E5x_Family_Data_Sheet_DS60001507-3107027.pdf

So if using this code, do not reboot more than 10,000 times.



