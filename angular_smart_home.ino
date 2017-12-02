/*  .
 *  angular webhost with index, script.js, action.php and status.php
 *  todo: don't need jsonp! Can set headers! see https://forum.arduino.cc/index.php?topic=476291.0
 *  able to update via wifi and respond to alexa requests
 *  also serves a simple dash that lets you request actions.
 *  uses wifi manager to set up wifi connections.
 *  uses corrected isDST algorithm.
 *  implementation: wifi relay
*/

/* based on https://github.com/kakopappa/arduino-esp8266-alexa-multiple-wemo-switch */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <functional>
#include "switch.h" 
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"

#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

/* my pages */
#include "index.html.h"          // the angular container page
#include "script.js.h"          // my scripts.

/* create dummy functions, to be overwritten by specifics.h if needed. */
void RunImplementationSetup(); //repeat this in specifics.h if needed.
void RunImplementationLoop();  //repeat this in specifics.h if needed.
void handleRoot();             //repeat this in specifics.h if needed.
void handleScript();           //repeat this in specifics.h if needed.
void handleFeatures();         //repeat this in specifics.h if needed.

// prototypes
boolean connectWifi();


boolean wifiConnected = false;

UpnpBroadcastResponder upnpBroadcastResponder;

Switch *first_device = NULL;
/*Switch *second_device = NULL;*/

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//time setup
static const char ntpServerName[] = "us.pool.ntp.org";
int timeZoneOffset = 0;     // GMT
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
void handleRoot();
void sendNTPpacket(IPAddress &address);
bool isDst(int day, int month, int dow); //defined below

time_t prevDisplay = 0; // when the digital clock was displayed

#include "specifics.h"

void setup(){
  Serial.begin(115200);

   
  // Initialise wifi connection
  WiFiManager wifiManager;

  wifiConnected = wifiManager.autoConnect(AP_NAME);
  //wifiManager.startConfigPortal(AP_NAME); //use to test wifi config capture.
  
  if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();
    
    // Define your switches here. Max 14
    // Format: Alexa invocation name, local port no, on callback, off callback
    first_device = new Switch(ALEXA_DEVICE_1, 81, FirstDeviceOn, FirstDeviceOff);
    /*second_device = new Switch(ALEXA_DEVICE_2, 82, SecondDeviceOn, SecondDeviceOff);*/

    //Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*first_device);
    /*upnpBroadcastResponder.addDevice(*second_device);*/

    Serial.println("Registering mDNS host"); 
    MDNS.begin(AP_NAME);

    httpUpdater.setup(&httpServer);
    httpServer.on("/", handleRoot);
    httpServer.on("/script.js", handleScript);
    httpServer.on("/action.php", handleAction);
    httpServer.on("/features.json",handleFeatures);
    httpServer.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", AP_NAME);

    Serial.println("Looking for NTP time signal");
    Udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
    Serial.println("waiting for sync");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);

    Serial.println("Running implementation setup");
    RunImplementationSetup();

    
    
    Serial.println("Entering loop");
  }
}

void loop(){

  yield(); //let the ESP8266 do its background tasks.
  
  //connect wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    
    return;
  }else{
      upnpBroadcastResponder.serverLoop();
      
      first_device->serverLoop();
      /*second_device->serverLoop();*/
      
      httpServer.handleClient();
  }
  RunImplementationLoop();
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime(){
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return 1 + secsSince1900 - 2208988800UL + timeZoneOffset * SECS_PER_HOUR; //1 is the adjustment for lag.
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address){
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

bool isDst(int day, int month, int dow) {
    /* Purpose: works out whether or not we are in UK daylight savings time SUMMER TIME.
     using day, month and dayOfWeek
     Expects: day (1-31), month(0-11), day of week (0-6)
     Returns: boolean true if DST
     */
    
    month++; //too tricky to do zero based.
    
    if (month < 3 || month > 10)  return false; //jan,feb, nov, dec are NOT DST. 
    if (month > 3 && month < 10)  return true;  //apr,may,june,july,august,september IS DST.

    //now the picky ones.
    int previousSunday = day - dow;
    
    if (month == 3) return previousSunday >= 25; //the most recent sunday was the last sunday in the month.
    if (month == 10) return previousSunday < 25; //the most recent sunday was not the last sunday in the month.

    return false; // something went wrong.
}

