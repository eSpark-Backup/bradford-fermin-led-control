# RGB Addressable LED Control Board
#### Client Name: Bradford Fermin

## Project Requirements
The board comprises of three PCBs being referred to as:
1. Main PCB
2. LED PCB
3. Switch PCB

#### Overview of Requirements
![image](https://github.com/user-attachments/assets/66f2b955-8a98-4729-8101-17f18c4334dd)

#### Concept Image
![image](https://github.com/user-attachments/assets/4a011b19-a8fe-4cc9-abb9-a0bda0428528)


### Main PCB Hardware Requirements
- The board should be based on an MCU that is capable of driving the Addressable RGB LED as well as having a USB control for programming and serial logging. Client preference is an RP2040.
- The board should have a JST for a limit switch that is used to trigger the RGB LED. The switch should be same as the RGB LED switch. The connector should be right-angled.
- The board should have a JST for a standard 3 wire RGB Addressable LED and a RESET button. The switch pitch should be kept the smallest pitch possible since the mating side also depends on our selection. The connector should be right-angled.
- The board should have an additional 3-pin JST as a spare that is top facing.
- The board should host a Male USB Type A for power and flashing.
- Overall power consumption of board cannot exceed 5V/1.5A (USB powered). There are only 3 pixels with each pulling a maximum of 60mA at 5V.
- There should also be a RESET switch, functionality of whose and type is TBD.
- The board size should be kept as small as possible.
- The 2 x JSTs (for LED and Switch PCBs) should be placed in a single line opposite to MCU.
- JLCPCB parts should be targeted for the BOM.

#### Concept Image
![image](https://github.com/user-attachments/assets/8f9727fd-d65a-4a83-af41-a2342a111ad8)

**Note: There is a discrepancy here between the image and the PRD since client is calling top facing connector as right-angled, so please ignore and consider what we consider right-angled.**

### LED PCB Hardware Requirements
- The board should have compatible mating connector to connect with the Main PCB.
- The board should have 3 x RGB LEDs placed in manner as shown in concept image below.
- The board should have a right-angled tactile switch acting as RESET button and placed in a manner shown in concept image below

#### Concept Image
![image](https://github.com/user-attachments/assets/48fca796-2894-41e3-82b1-fb9e0aba5a79)


### Switch PCB Hardware Requirements
- This board will only have a [limit switch](https://www.lcsc.com/product-detail/Limit-Switches_XKB-Connection-DM1-135P_C480264.html) and a Red LED.
- The board will have a mating JST connector compatible with the Main PCB.
- The Red LED should only light up when the limit switch is pressed down. When limit switch is not pressed, the LED will turn OFF.

#### Concept Image
![image](https://github.com/user-attachments/assets/af7a636a-922e-41c5-9183-a42d10455e03)

**Note: There is a discrepancy here between the image and the PRD since client is specifying JST connector type. We will go, however, with what's compatible and uniform with the rest of the boards.**

### Firmware Sequence for Hardware Design
- Nothing happens on single click
- 2 clicks in quick succession on limit switch triggers below NeoPixel animation function:
    - Function Name: Printing
    - Color: Green
    - Effect: Slow pulsing (1-second fade in and out) to indicate active progress.
- 3 clicks in quick succession on limit switch triggers below NeoPixel animation function:
    - Function Name: Print Finished
    - Color: Blue
    - Effect: Quick double pulse every few seconds (take 5 seconds for now) to show completion without urgency.
- 4 clicks in quick succession on limit switch triggers below NeoPixel animation function:
    - Function Name: Idle (Ready to Print)
    - Color: White
    - Effect: Steady glow to signify readiness and neutrality.
- 5 clicks in quick succession on limit switch triggers below NeoPixel animation function:
    - Function Name: Pause
    - Color: Yellow
    - Effect: Slow blinking (0.5 seconds on, 0.5 seconds off) to indicate a temporary halt
- 6 clicks in quick succession on limit switch triggers below NeoPixel animation function:
    - Function Name: Error
    - Color: Red
    - Effect: Rapid flashing (0.25 seconds on, 0.25 seconds off) to grab attention for troubleshooting.
- The "idle" state should be the default state the device is in when powered on
- Clicking the button on the neopixel PCB will take the printer from the "pause" state to the "printing" state (the button should work ONLY when in the "pause" state)
- RESET button clears current function state and brings board back to Idle state.
