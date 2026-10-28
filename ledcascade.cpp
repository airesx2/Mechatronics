#include <Arduino.h>

// ~~~~~Pins~~~~~
const int SWITCH_PIN = A1; //microswitch: COM -> A1, NO -> GND
const int LED_PINS[] = {7, 8, 9, 10, 11}; //firing order; last one faces the LDR
const int NUM_LEDS = sizeof(LED_PINS) / sizeof(LED_PINS[0]);
const int LDR_PIN = A0; //LDR / Photoresistor
const int ENA = 2; // L298N ENA
const int IN3 = 4; // L298N IN3
const int IN4 = 3; // L298N IN4

// ~~~~~Tuning~~~~~
// unsigned int disables ability to store neg int
const unsigned long STEP_MS = 1000; //delay between LEDs
const int LIGHT_MARGIN = 10; //brightness jump btw baseline and measured light
const int MOTOR_SPEED  = 200; 
const int MOTOR_TIMEINTERVAL = 15000; //motor delay to turn off


//booleans to check each triggered stage
boolean switchTriggered = false;
boolean photoresistorTriggered = false;
int baseline = 0;


//start reading LDR 
int readLDR() {
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(LDR_PIN);
  return sum/8; //average read to combat noise 
}

//check if switch actively pressed 
boolean switchPressed() {
  if (digitalRead(SWITCH_PIN) == LOW) {
    delay(30);
    return digitalRead(SWITCH_PIN) == LOW;
  }
  return false;   
}

//IN
void motorOn() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  digitalWrite(ENA, HIGH);
}

void motorOff() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENA, LOW);
}


void resetAll() {
  //set motors low and make all booleans false 
  motorOff();
  for (int i = 0;i < NUM_LEDS; i++) digitalWrite(LED_PINS[i], LOW);
  switchTriggered = false;
  photoresistorTriggered = false;
  Serial.println("Reset");
}

void setup() {
  Serial.begin(115200); //Serial begin as always 
  //pinmode setup
  pinMode(SWITCH_PIN, INPUT_PULLUP); //switch pin
  for (int i = 0; i < NUM_LEDS; i++) pinMode(LED_PINS[i], OUTPUT); //all led pins
  //L298N motor pins
  pinMode(ENA, OUTPUT); 
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  resetAll();
}

void loop() {
  //Stage 1: wait for switch --> run LED cascade
  if (!switchTriggered) { //needs to start off switch not triggered 
    if (switchPressed()) {
      switchTriggered=true;
      baseline = readLDR(); //led baseline
      //print to console
      Serial.print("Switch triggered! Baseline = ");
      Serial.println(baseline);

      //led cascade loop: turn high --> delay --> next
      for (int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(LED_PINS[i], HIGH);
        delay(STEP_MS);
      }
    }
    return;
  }

  //Stage 2: wait for last LED to hit the LDR
  if (!photoresistorTriggered) {
    int light = readLDR();
    //print LDR info
    Serial.print("LDR: ");
    Serial.println(light);

    // compare with led baseline 
    if (abs(light - baseline) > LIGHT_MARGIN) {
      photoresistorTriggered = true;
      Serial.println("Photoresistor triggered --> motor ON");
      motorOn();
      delay(MOTOR_TIMEINTERVAL);
      motorOff();
    }
  }

  delay(50); //delay btw each cascade check 
}
