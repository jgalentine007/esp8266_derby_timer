#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

const char *ssid = "Derby";
const char *password = "derbytimer";
ESP8266WebServer server(80);

const int numLanes = 4;
const int lanePin[] = {14, 12, 13, 15};
const int ledPin =  0;
const int piezoPin = 2;   // disconnect from ground when uploading via toggle switch

volatile bool raceStarted = false;
unsigned long startTime = 0;
volatile int carsFinished = 0;

volatile unsigned long laneTime[numLanes];
volatile bool laneFinished[numLanes];
volatile int finishedOrder[numLanes];

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

void handleRoot() {
  String response = "<HTML><BODY><font size=\"6\">\n";
  
  for(int i=0; i<numLanes; i++)
    response += "Lane " + String(i) + ":" + String((laneTime[i] - startTime) / 1000.00) + "s<br>\n";

  if (raceStarted)
    response += "<br>Race Started<br><br>\n";
  else
    response += "<br>Race Stopped<br><br>\n";
    
  response += "<a href='/NewRace'>NEW RACE</a> - <a href='/'>REFRESH</a>";
  response += "</font></BODY></HTML>";
  server.send(200, "text/html", response);
}

void newRace() {
  startRace();
  server.sendHeader("Location", "/", true);
  server.send(307, "text/plain", "");
}

void setup() {
  for (int i=0; i<numLanes; i++){
    finishedOrder[i] = 0;
    laneFinished[i] = true;
    laneTime[i] = 0;
  }
  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/NewRace", newRace);
  server.begin();
  Serial.println("HTTP server started");
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  
  // initialize the pushbutton pin as an input:
  pinMode(lanePin[0], INPUT_PULLUP);
  pinMode(lanePin[1], INPUT_PULLUP);
  pinMode(lanePin[2], INPUT_PULLUP);
  pinMode(lanePin[3], INPUT); // GPIO 15 has a PULL DOWN resistor attached to it (switch must be connected to +3v3)

  digitalWrite(lanePin[0], HIGH);
  digitalWrite(lanePin[1], HIGH);
  digitalWrite(lanePin[2], HIGH);

  attachInterrupt(digitalPinToInterrupt(lanePin[0]), finish0, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[1]), finish1, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[2]), finish2, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[3]), finish3, RISING); // GPIO 15 has a PULL DOWN resistor attached to it (switch must be connected to +3v3)

  digitalWrite(ledPin, HIGH); // Turn LED OFF (it is opposite)  

  alpha4.begin(0x70);  // pass in the address

  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  
  alpha4.clear();
alpha4.writeDisplay();

}

void loop() {
  server.handleClient();
}

void blinkLed(){
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
}

void alpha4Clear(){
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(4, 0x0);
  alpha4.writeDisplay();
}

void startRace(){
  alpha4Clear();
  // 3 2 1 go beep
  //blinkLed();
  //blinkLed();
  //blinkLed();
  //digitalWrite(ledPin, LOW);
  alpha4.writeDigitAscii(0, '3');
  alpha4.writeDisplay();
  tone(piezoPin, 3700, 500);
  delay(1000);
  alpha4.writeDigitAscii(1, '2');
  alpha4.writeDisplay();
  tone(piezoPin, 3700, 500);
  delay(1000);
  alpha4.writeDigitAscii(2, '1');
  alpha4.writeDisplay();
  tone(piezoPin, 3700, 500);  
  delay(1000);
  startTime = millis();
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitAscii(1, 'G');
  alpha4.writeDigitAscii(2, 'O');
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDisplay();
  tone(piezoPin, 4100, 2000);
  delay(2000);
  alpha4Clear();  
  raceStarted = true;
  
  carsFinished = 0;
  for (int i=0; i<numLanes; i++){
    laneTime[i] = startTime;
    laneFinished[i] = false;
  }
}

void finish(int lane){
  if (raceStarted && !laneFinished[lane]){
    laneTime[lane] = millis();
    laneFinished[lane] = true;
    carsFinished++;
   
    alpha4.writeDigitAscii(lane, carsFinished + '0');    
    alpha4.writeDisplay();
    finishedOrder[lane] = carsFinished;
    if (carsFinished == numLanes){
      raceStarted = false;
      digitalWrite(ledPin, HIGH);
    }
  }
}

void finish0() {
  finish(0);
}

void finish1() {
  finish(1);
}

void finish2() {
  finish(2);
}

void finish3() {
  finish(3);
}


