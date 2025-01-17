# FLASHING GUIDE FOR RASP PI pico



1. Connect the rasp pi pico via micro usb cable ,while connecting make sure to press down the BOOTSEL Button to get the pico in the flash mode.
2. the pico will appear as a drive(RPI-RP2) in the explorer
3. paste the .uf2 file in the RPI-RP2 drive 
4. after placing the file , the rasp pi will be flased an will exit the BOOTSEL mode 
5. the pico will be detected as a COM device after this which can be used to monitor logs if required



*NOTE : if the pico is not being detected as a device. download and install Zadig ( https://zadig.akeo.ie/ ) and select the RPI-RF2 interface and install the WinUSB32 driver.*

*if a problem is encountered the user can refer to the "Getting Started with pico" documentation ( https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf ).*