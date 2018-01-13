#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>

const char *ssid = "Vzg";
const char *password = "freedom34";
ESP8266WebServer server(80);

const int numLanes = 4;
const int lanePin[] = {14, 12, 13, 5};
const int ledPin =  0;
const int startPin = 4;
const int readyPin = 2;

volatile bool raceStarted = false;
volatile bool raceReady = true;
unsigned long startTime = 0;
volatile int carsFinished = 0;

volatile unsigned long laneTime[numLanes];
volatile bool laneFinished[numLanes];
volatile int finishedOrder[numLanes];

void handleRoot() {
  String response = "<HTML><BODY><font size=\"6\">\n";
  
  for(int i=0; i<numLanes; i++)
    response += "Lane " + String(i) + ":" + String((laneTime[i] - startTime) / 1000.00) + "s<br>\n";

  if (raceReady)
    response += "<br>Race Ready<br>\n";
  else
    response += "<br>Race NOT Ready<br>\n";
    
  if (raceStarted)
    response += "<br>Race Started<br><br>\n";
  else
    response += "<br>Race Stopped<br><br>\n";
    
  response += "<a href='/ReadyRace'>READY RACE</a> - <a href='/NewRace'>NEW RACE</a> - <a href='/EndRace'>END RACE</a> - <a href='/'>REFRESH</a>";
  response += "</font></BODY></HTML>";
  server.send(200, "text/html", response);
}

void newRace() {
  startRace();
  server.sendHeader("Location", "/", true);
  server.send(307, "text/plain", "");
}

void apiNewRace() {
  startRace();
  server.send(200, "application/json", "");
}

void endRace() {
  finishAll();
  server.sendHeader("Location", "/", true);
  server.send(307, "text/plain", "");
}

void apiEndRace() {
  finishAll();
  server.send(200, "application/json", "");
}

void readyRace() {
  readyReadyRace();
  server.sendHeader("Location", "/", true);
  server.send(307, "text/plain", "");
}

void apiReadyRace() {
  readyReadyRace();
  server.send(200, "application/json", "");
}

void apiResults() {
  String response = "{\"LaneTimes\":[";
  
  for(int i=0; i<numLanes; i++){
    response += String((laneTime[i] - startTime) / 1000.00);
    if(i < numLanes-1)
      response += ",";
  }

  response += "], \"RaceStarted\":";

  if (raceStarted)
    response += "true";
  else
    response += "false";

  response += "}";
  server.send(200, "application/json", response);
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
  WiFi.begin(ssid,password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/NewRace", newRace);
  server.on("/EndRace", endRace);
  server.on("/ReadyRace", readyRace);
  server.on("/api/NewRace", apiNewRace);
  server.on("/api/EndRace", apiEndRace);
  server.on("/api/ReadyRace", apiReadyRace);
  server.on("/api/Results", apiResults);
  
  server.begin();
  Serial.println("HTTP server started");
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  pinMode(readyPin, OUTPUT);
    
  // initialize the lane pins as inputs
  pinMode(lanePin[0], INPUT_PULLUP);
  pinMode(lanePin[1], INPUT_PULLUP);
  pinMode(lanePin[2], INPUT_PULLUP);
  pinMode(lanePin[3], INPUT_PULLUP);
  pinMode(startPin, INPUT_PULLUP);
  
  digitalWrite(lanePin[0], HIGH);
  digitalWrite(lanePin[1], HIGH);
  digitalWrite(lanePin[2], HIGH);
  digitalWrite(lanePin[3], HIGH);
  digitalWrite(startPin, HIGH);
  
  attachInterrupt(digitalPinToInterrupt(lanePin[0]), finish0, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[1]), finish1, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[2]), finish2, FALLING);
  attachInterrupt(digitalPinToInterrupt(lanePin[3]), finish3, FALLING); 
  attachInterrupt(digitalPinToInterrupt(startPin), startRace, FALLING);
  
  digitalWrite(ledPin, HIGH); // Turn LED OFF (it is opposite)  
  digitalWrite(readyPin, LOW);
}

void loop() {
  server.handleClient();
}


void startRace(){
  if (!raceStarted && raceReady){
    digitalWrite(ledPin, LOW); // Turn LED ON - race is running
    digitalWrite(readyPin, HIGH);
    startTime = millis();
    raceStarted = true;
    raceReady = false;
    
    carsFinished = 0;
    for (int i=0; i<numLanes; i++){
      laneTime[i] = startTime;
      laneFinished[i] = false;
    }

    Serial.println("Race started.");
  }
}

void readyReadyRace() {
  raceReady = true;
  digitalWrite(readyPin, LOW);  
}

void finish(int lane){
  if (raceStarted && !laneFinished[lane]){
    laneTime[lane] = millis();
    laneFinished[lane] = true;
    carsFinished++;
    finishedOrder[lane] = carsFinished;
    char buffer[20];
    sprintf(buffer, "Lane %d finished.", lane);
    Serial.println(buffer);
    
    if (carsFinished == numLanes){
      raceStarted = false;
      digitalWrite(ledPin, HIGH);
      Serial.println("Race complete.");
    }
  }
}

void finishAll(){
  for(int i=0; i<numLanes; i++)
    finish(i);
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
