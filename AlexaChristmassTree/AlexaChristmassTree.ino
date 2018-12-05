

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"

#define TRIGGER_PIN 0
#define RELAY_PIN 12


//on/off callbacks
bool officeLightsOn();
bool officeLightsOff();

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *office = NULL;

bool isOfficeLightsOn = false;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n Starting");

  pinMode(TRIGGER_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);


}
bool firstRun = true;

void loop()
{
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;

    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);

    if (!wifiManager.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.println(WiFi.localIP());
  }

  if (firstRun && WiFi.status() == WL_CONNECTED) {
    firstRun = false;
    upnpBroadcastResponder.beginUdpMulticast();

    // Define your switches here. Max 10
    // Format: Alexa invocation name, local port no, on callback, off callback
    office = new Switch("sonoff", 85, officeLightsOn, officeLightsOff);

    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*office);
  }
  if( WiFi.status() == WL_CONNECTED )
  {
     upnpBroadcastResponder.serverLoop();
    office->serverLoop();
  }
 
}

bool officeLightsOn() {
  Serial.println("Switch 1 turn on ...");

  isOfficeLightsOn = true;
  digitalWrite(RELAY_PIN, HIGH);
  return isOfficeLightsOn;
}

bool officeLightsOff() {
  Serial.println("Switch 1 turn off ...");

  isOfficeLightsOn = false;
  digitalWrite(RELAY_PIN, LOW);
  return isOfficeLightsOn;
}


