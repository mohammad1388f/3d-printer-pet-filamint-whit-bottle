/*
A0 ----- ntc ----vcc
      I
      I
      R1  ------- gnd
*/

#include <WiFi.h>//#include <ESP8266WiFi.h>
#include <WebServer.h>//#include <ESP8266WebServer.h>
#include <Arduino.h>
#include "BasicStepperDriver.h"
const char* ssid = "filament";
const char* password = "123456789";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
BasicStepperDriver stepper(100, 12, 0); 
int speed = 50; 
int RPM = 200; 
int temp = 270; 

const int analogPin = A0;   
const float Vcc = 3.3;     
const float R1 = 10000.0; 


const float A = 0.001129148;
const float B = 0.000234125;
const float C = 0.0000000876741;

void setup() {
  Serial.begin(115200);
  pinMode(34, INPUT); 
  pinMode(0, OUTPUT);  
  pinMode(2, OUTPUT);    
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  Serial.print("Connect to My access point: ");
  Serial.println(ssid);

  server.on("/", handle_root);

  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop() {
   int sensorValue = analogRead(analogPin); 
  float Vin = (sensorValue * Vcc) / 1023.0;

  float Rth = (Vcc * R1) / Vin - R1; 
  float logRth = log(Rth);
  float Tc = 1.0 / (A + (B * logRth) + (C * logRth * logRth * logRth)); 

  server.handleClient(); 
  if (server.hasArg("speed") && server.hasArg("temp")) {
    speed = server.arg("speed").toInt();
    temp = server.arg("temp").toInt(); 
    
  }  
      RPM = map (speed,0,100,0,200);
      stepper.begin(RPM , 32);
      stepper.rotate(360);
      stepper.move(-100 * 32);
      if (Tc<temp)
    {
      digitalWrite(2, HIGH);      
    }     
      else 
     {
      digitalWrite(2, LOW); 
     }          
}

const char MAIN_page[] PROGMEM = R"=====(
<html>
<body>
  <h3>Control the project</h3>
  <form action="/" method="get">
    Speed: <input type="number" name="speed" min="0" max="100" value="50"><br><br>
    Temperature: <input type="number" name="temp" min="0" max="330" value="270"><br><br>
    <input type="submit" value="Submit">
  </form>
  <p>the temp is : @@ANALOG@@</p> <!-- Display analog value on webpage -->
</body>
</html>
)=====";

void handle_root() {
  String page = String(MAIN_page); 

  page.replace("@@ANALOG@@", String(Tc)); 
  server.send(200, "text/html", page);
}

