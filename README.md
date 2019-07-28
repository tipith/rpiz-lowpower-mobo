# Motherboard

Purpose of this PCB is have a low-power way of sending SMS messages once an external trigger is detected. For example, it can be used as a an detector for door open events.

Features
- 18650 battery holder
- Attiny1604 power manager
- External trigger connector
- Nokia phone connector (FBUS + battery size indicator + battery temp)
- 2-way communication between PM and RPi

![alt text](https://raw.githubusercontent.com/tipith/rpiz-lowpower-mobo/master/docs/pcb_3d.png)

# Power manager (attiny1604)

tbd

# SMS sender (RPi Zero)

## Instructions

- /boot
   - add file: sshe
   - add file: wpa_supplicant.conf
   - add file: trapsms.ini
- /boot/config.txt
   - add "dtoverlay rpi3-disable-bt"
   - uncomment and modify "dtparam=i2c_arm=on,i2c_arm_baudrate=50000"
-  /boot/cmdline.txt
   - remove "console=serial0,115200 console=tty1"
- shell commands
   - sudo systemctl disable hciuart
   - sudo apt update && sudo apt upgrade
   - sudo apt install git gammu python3-gammu
- install pigpio
  - sudo apt-get install pigpio
  - wget https://raw.githubusercontent.com/joan2937/pigpio/master/util/pigpiod.service
  - sudo cp pigpiod.service /etc/systemd/system
  - sudo systemctl enable pigpiod.service
  - sudo systemctl start pigpiod.service
- run
   - python3 rpi_smssender/sms_sender.py 
