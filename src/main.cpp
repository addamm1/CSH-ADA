#include <Arduino.h>
#include <SoftwareSerial.h>

#define CASHINPUT 2
#define COININPUT 3
#define CASHINHIBIT 4
#define COININHIBIT 5
#define PULSES_PER_DOLLAR 1

int countPulses(int, int);

SoftwareSerial mySerial(COININPUT, 13);
String input;
char newInput;


void setup() {
  Serial.begin(9600); // USB Serial back to RPI
  mySerial.begin(4800); // Recieve only serial from Coin Acceptor
  pinMode(CASHINPUT, INPUT_PULLUP);
  pinMode(CASHINHIBIT, OUTPUT);
  pinMode(COININHIBIT, OUTPUT);
  digitalWrite(COININHIBIT, LOW);
  digitalWrite(CASHINHIBIT, LOW);
}

void loop() {
  //If there is a new message over USB serial, check it against known commands
  if(Serial.available()){
    newInput = Serial.read();
    if(newInput >= 0x41 && newInput <= 0x5A){
      input += newInput;
    }
    if(newInput == '\n'){
      if(input == "INHIBIT"){
        digitalWrite(CASHINHIBIT, LOW);
        digitalWrite(COININHIBIT, LOW);
        //Serial.println("INHIBITING");
      }
      else if(input == "ACCEPT"){
        digitalWrite(CASHINHIBIT, HIGH);
        digitalWrite(COININHIBIT, HIGH);
        //Serial.println("ACCEPTING");
      }
      input = "";
    }
  }

  if(digitalRead(CASHINPUT) == 0){
    Serial.println("0");
  }

  // // Get data from bill acceptor if available
  if(digitalRead(CASHINPUT) == LOW){
    int dollarsInserted = countPulses(PULSES_PER_DOLLAR, CASHINPUT);
    switch (dollarsInserted)
    {
    case 20:
      Serial.print("TWENTY_DOLLAR\n");
      break;
    case 10:
      Serial.print("TEN_DOLLAR\n");
      break;
    case 5:
      Serial.print("FIVE_DOLLAR\n");
      break;
    case 1:
      Serial.print("ONE_DOLLAR\n");
      break;
    default:
      break;
    }
  }
  
  // Get data from Coin acceptor if available
  if(mySerial.available()){
    int centsInserted = (int)mySerial.read();
    switch (centsInserted)
    {
    case 25:
      Serial.print("QUARTER\n");
      break;
    case 10:
      Serial.print("DIME\n");
      break;
    case 5:
      Serial.print("NICKEL\n");
      break;
    case 1:
      Serial.print("PENNY\n");
      break;
    default:
      break;
    }
  }
}


// count the number of pulses recieved
int countPulses(int pulsePer, int inputNumber){
  int pulses = 0; // Number of pulses recieved
  unsigned long lastTime = 0; // Millis of the rising edge of the previous pulse
  bool last = HIGH; // If the last event was a rising or falling edge
  while(true){
    int pulse = digitalRead(inputNumber); // Current state of the input pulse signal
    if(pulse == LOW && last == HIGH){ // Falling edge
        pulses++;
        last = LOW;
    }

    else if(pulse == LOW && last == LOW){ // Still LOW
      continue;
    }

    else if(pulse == HIGH && last == LOW){ // Rising Edge
      last = HIGH;
      lastTime = millis();
      continue;
    }

    else if(pulse == HIGH && last == HIGH){ // Still HIGH
      if((millis() - lastTime) > 200){ // if it has been more than 200ms since the last rising edge, assume done pulsing
        return pulses / pulsePer;
      }
      continue;
    }
  }
}