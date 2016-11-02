#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "Derby";
ESP8266WebServer server(80);

const int numLanes = 4;
const int lanePin[] = {14, 12, 13, 15};
const int ledPin =  0;

volatile bool raceStarted = false;
unsigned long startTime = 0;
volatile int carsFinished = 0;

volatile unsigned long laneTime[numLanes];
volatile bool laneFinished[numLanes];
volatile int finishedOrder[numLanes];


void handleRoot() {
  String response = "<HTML><BODY>\n";
  
  for(int i=0; i<numLanes; i++)
    response += "Lane " + String(i) + ":" + String((laneTime[i] - startTime) / 1000.00) + "s<br>\n";
  response += "Race Started:" + String(raceStarted) + "<br>\n";
  response += "<input type=\"button\" onclick=\"location.href='/NewRace';\" value=\"New Race\"/>";
  response += "</BODY></HTML>";
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
  WiFi.softAP(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/NewRace", newRace);
  server.begin();
  Serial.println("HTTP server started");
  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  
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

void startRace(){
  // 3 2 1 go beep
  blinkLed();
  blinkLed();
  blinkLed();
  digitalWrite(ledPin, LOW);
  
  raceStarted = true;
  startTime = millis();
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


