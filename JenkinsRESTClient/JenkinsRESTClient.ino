// Arduino Jenkins REST client
//
// Copyright: GPL V2
// See http://www.gnu.org/licenses/gpl.html
// by https://github.com/andy1024
// based on EtherCard examples

#include <EtherCard.h>

static byte mymac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };

byte Ethernet::buffer[200];
static uint32_t timer;

const char website[] PROGMEM = "192.168.1.21";

// request callbacke
static void my_callback (byte status, word off, word len) {
  Serial.println(">>> in callback method");
  Ethernet::buffer[off+200] = 0; //cut-off
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println(">>> after callback method");
}

void setup () {
  Serial.begin(9600);
  Serial.println(F("\n[Jenkins REST Client]"));

  Serial.print("MAC assigned: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.println();
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println(F("Ethernet controller initialization failure"));

  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failure"));
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  if (!ether.dnsLookup(website))
    Serial.println("DNS failure");
    
  ether.printIp("SRV: ", ether.hisip);

}

void loop () {
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {
    timer = millis() + 30000;
    Serial.println();
    Serial.print("<<< REQ ");
    Serial.println(F("Before browseUrl"));
    ether.browseUrl(PSTR("/"), "jenk2", website, my_callback);
    Serial.println(F("After browseUrl"));
  }
}
