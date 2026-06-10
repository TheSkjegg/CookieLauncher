// Description: 
//    Plain Cookie Launcher code
//    Outputs: two RC servos, two N20 DC motors (via DRV8833 motor driver) and a button led. 
//    Inputs: Pushbutton, two optical end stop switches
//
// Usage:
//    Developed on the Seed Studio XIAO SAMD21 board, but should be compatible with all "Arduino UNO" boards.
//    Some RC servo position calibration might be required
//    Make sure to adjust delays marked ***ADJUST*** below
//
// Author: 
//    The Skjegg 10/06/2026
//
// Versions:
//    v001: First version

#include <Servo.h>

// Seeed Studio XIAO IO mapping
const int MA1 = 0;
const int MA2 = 1;
const int MB1 = 3;
const int MB2 = 4;
const int buttonLed = 5;
const int button = 6;
const int armStop = 8;
const int fillStop = 7;
const int guideServo = 9;
const int hatchServo = 10;

// Hatch servo positions
const int hatchOpen = 75;
const int hatchGlip = 100;
const int hatchClosed = 170;

// Guide servo positions
const int guideOpen = 55;
const int guideClosed = 75;

unsigned long timeout = 12000;
unsigned long startTime = 0;
int ledState = LOW;
unsigned long prevMillis = 0;
const int ledBlink = 1000;

Servo hatch;
Servo guide;

volatile bool buttonPressed = false;

void setup() {

  hatch.attach(hatchServo);
  hatch.write(hatchClosed);
  delay(1000);

  guide.attach(guideServo);
  guide.write(guideOpen);

  pinMode(buttonLed, OUTPUT); 

  pinMode(MA1, OUTPUT);
  pinMode(MA2, OUTPUT);
  pinMode(MB1, OUTPUT);
  pinMode(MB2, OUTPUT);

  pinMode(fillStop, INPUT);
  pinMode(armStop, INPUT);
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING); // Interrupt on launch button
}

void handleButtonInterrupt(){ // Interrupt routine
  buttonPressed = true;
}


void launch(){
      for(int i = hatchClosed;i>hatchOpen;i--){ // Open hatch
        hatch.write(i);
        delay(10);
      }
      guide.write(guideClosed); // Close guide

      digitalWrite(MA2, HIGH); // Start main motor (use MA1 to change motor direction)
      startTime = millis();
      delay(3500);
      hatch.write(hatchClosed); // Close hatch
      while(!digitalRead(armStop)){ // Run motor until armed again
        if (millis() - startTime > timeout) // Stop motor if timeout limit is reached
          break;
      }
      delay(800); // ***ADJUST*** to stop at correct position, link at bottom (might be a bit battery voltage dependent)
      digitalWrite(MA2, LOW); // Stop main motor (use MA1 to change motor direction)

      hatch.write(hatchGlip); // Makes a small hatch noise, just for fun
      delay(100);
      hatch.write(hatchClosed);     
      
      guide.write(guideOpen); // Open guide

      digitalWrite(MB2, HIGH);  // Start magazine motor (use MB2 to change motor direction)
      startTime = millis();
      delay(2000);
      while(analogRead(fillStop) < 120){ // Run motor until filled again
        if (millis() - startTime > timeout) // Stop motor if timeout limit is reached
          break;
      }
      delay(300); // ***ADJUST*** to make sure cookie rolls out of magazine properly
      digitalWrite(MB2, LOW); // Stop magazine motor (use MB2 to change motor direction)

      hatch.write(hatchGlip); // Makes a small hatch noise, just for fun
      delay(100);
      hatch.write(hatchClosed);  
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - prevMillis > ledBlink){ // Non-blocking button led blink
    prevMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(buttonLed, ledState);
  }

  delay(20);

  if(buttonPressed){
    launch();
    buttonPressed = false;
  }

}