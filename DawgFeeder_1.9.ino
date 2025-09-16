/* MECH 307 Project Code: Dawg Feeder

   This code serves as the control system for an automated pet feeder, designed to respond to IR remote commands. Its primary functionalities include:

   - System Activation: The system can be activated or deactivated using IR remote buttons.
   - Food Dispensing: Utilizing a photoresistor, the system dispenses food when the light falls below a certain threshold, indicated by the 'desiredValue'.
   - Water Management: IR commands manage water supply, turning pumps on or off based on remote button presses.
   - Hatch Control: IR commands control the opening and closing of the feeder hatch using servo motors.

   Additionally, an emergency stop feature is integrated into the system, triggered by a physical button (ESTOP), immediately halting all operations when activated.

   Components utilized in the system:
   - LCD for displaying system status and messages.
   - IR receiver for receiving remote commands.
   - Servo motors for controlling hatch movement.
   - LEDs and buzzer for visual and auditory feedback.
   - Various pins configured for component interfacing (e.g., LEDs, pumps, etc.).

   Contributors:
   - Haydn Simpson
   - Nathan Pardoen
   - Jack Cavo
   - Justin Rudrow

   Created: 10/27/2023
   Last Revision: 12/3/2023
*/
#include <LiquidCrystal.h>
#include "IRremote.h"
#include <Servo.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
#define IR_RECEIVE_PIN 23
#define IR_BUTTON_1 12
#define IR_BUTTON_2 24
#define IR_VOLUMEUP 70
#define IR_VOLUMEDOWN 21
#define IR_UP 9
#define IR_DOWN 7

const int ESTOP = 45;
volatile bool emergencyStop = false;

Servo myServo1;
Servo myServo2;
const int servo_pin_1 = 50;
const int servo_pin_2 = 51;

const int buzzer = 2;
const int LED_RED = 5;
const int LED_BLUE = 4;
const int PUMPONE = 34;
const int PUMPTWO = 38;
const int LED_GREEN = 3;
bool ledOn = false;
int blinkCount = 0;

int analog_input = A0;
bool systemActivated = false;

const int desiredValue = 500;
//bool photoSensorActivated = false;
//bool foodDispensing = false;
//bool isThirsty = false;

void emergencyStopInterrupt() {
  emergencyStop = true; // Set the emergency stop flag when the interrupt is triggered
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(PUMPONE, OUTPUT);
  pinMode(PUMPTWO, OUTPUT);
  pinMode(ESTOP, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Dawg Feeder");

  IrReceiver.begin(IR_RECEIVE_PIN);
  myServo1.attach(servo_pin_1);
  myServo2.attach(servo_pin_2);
  myServo1.write(0);
  myServo2.write(80);
  pinMode(analog_input, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(ESTOP), emergencyStopInterrupt, HIGH);
}


void loop() {

  int photoresistorValue = analogRead(analog_input);

  if (digitalRead(ESTOP) == HIGH) {
    emergencyStop = true;
  }

  if (emergencyStop) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("E-Stop Engaged");
    delay (1000);
    // Perform actions for emergency stop
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    myServo1.write(-90);
    myServo2.write(90);
    digitalWrite(PUMPONE, LOW);
    digitalWrite(PUMPTWO, LOW);
    systemActivated = false;

    // Blink LED for indication
    blinkCount = 0;
    ledOn = true;
    while (ledOn) {
      digitalWrite(LED_RED, HIGH);
      delay(500);
      digitalWrite(LED_RED, LOW);
      delay(500);
      blinkCount++;
      if (blinkCount >= 1) {
        ledOn = false;
        digitalWrite(LED_RED, LOW);
        emergencyStop = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dawg Feeder");
      }
    }
  }

  if (IrReceiver.decode()) {
    IrReceiver.resume();
    int command = IrReceiver.decodedIRData.command;

    if (command == IR_BUTTON_1) {
      systemActivated = true; // Activate the system
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Auto Food: ON");
    }

    if (command == IR_BUTTON_2) {
      systemActivated = false; // Deactivate the system
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dawg Feeder");
    }

    if (systemActivated) {
      if (photoresistorValue <= desiredValue) {
        digitalWrite(LED_RED, HIGH);
        myServo1.write(90);// Move servos 90 degrees
        myServo2.write(-90);
        delay(5000); // Wait for 10 seconds
        digitalWrite(LED_RED, LOW);
        myServo1.write(-90);// Move servos back to original position
        myServo2.write(90);
        
      }

    }

    if (command == IR_VOLUMEUP) {
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(PUMPONE, HIGH);
      digitalWrite(PUMPTWO, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thirsty???");
    }
    if (command == IR_VOLUMEDOWN) {
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(PUMPONE, LOW);
      digitalWrite(PUMPTWO, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dawg Feeder");
    }
    if (command == IR_UP) { //move servos 80 degrees to open
      myServo1.write(80);
      myServo2.write(-80);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Food Time!!!");
      blinkCount = 0; // Reset the blink counter
      ledOn = true;
      while (ledOn) {
        digitalWrite(LED_RED, HIGH);  // Turn on the LED
        digitalWrite(buzzer, HIGH);
        delay(1000);                     // Wait for 1000 milliseconds
        digitalWrite(LED_RED, LOW);
        digitalWrite(buzzer, LOW);// Turn off the LED
        delay(1000);                     // Wait for 1000 milliseconds
        blinkCount++; // Increment blink count

        if (blinkCount >= 2) { // Check if blinked for 10 cycles
          ledOn = false; // Turn off the LED blinking
          digitalWrite(LED_RED, LOW); // Turn off the LED
          digitalWrite(buzzer, LOW);
        }
      }
    }
    if (command == IR_DOWN) { // Move servos to close hatch (-80 degrees)
      myServo1.write(-80);
      myServo2.write(80);
      digitalWrite(LED_GREEN, HIGH);
      delay(5000); // Wait for 5 seconds
      digitalWrite(LED_GREEN, LOW); // Turn off the green LED
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dawg Feeder");
    }
  }
}
