# RGB Addressable LED Control Board
#### Client Name: Bradford Fermin

## Project Requirements

### Hardware Requirements
- The board should be based on an MCU that is capable of driving the Addressable RGB LED as well as having a USB control for programming and serial logging. Client preference is an RP2040.
- The board should have a 2-pin JST for a limit switch that is used to trigger the RGB LED.
- The board should have a 3-pin JST for a standard 3 wire RGB Addressable LED.
- The board should host a Male USB Type A for power and flashing.
- Overall power consumption of board cannot exceed 5V/1.5A (USB powered). There are only 3 pixels with each pulling a maximum of 60mA at 5V.
- There should also be a RESET switch, functionality of whose and type is TBD.
- The board size should not exceed Seed Studio's Xiao.
- JLCPCB parts should be targeted for the BOM.

#### Concept Image
![image](https://github.com/user-attachments/assets/4a011b19-a8fe-4cc9-abb9-a0bda0428528)

### Firmware Sequence for Hardware Design
- 1 click on limit switch triggers first NeoPixel animation function.
- 2 clicks in quick succession trigger second function.
- 3 clicks in quick succession trigger third function.
- RESET button clears current function state.
