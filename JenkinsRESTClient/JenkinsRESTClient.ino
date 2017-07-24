// Arduino Jenkins REST client
//
// Copyright: GPL V2
// See http://www.gnu.org/licenses/gpl.html
// by https://github.com/andy1024
// based on EtherCard and NeoPixel examples

#include <EtherCard.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

static byte mymac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };

#define MAX_BUF 200

byte Ethernet::buffer[MAX_BUF];
static uint32_t timer;

#define DELAY_TIME 50

#define REQUEST_TIMEOUT 30000

//NeoPixel definitions
#define PIN            9

//How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1

const char website[] PROGMEM = "192.168.1.21";

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// request callback
static void my_callback (byte status, word off, word len) {
  Serial.println(">>> in callback method");
  Ethernet::buffer[off+MAX_BUF] = 0; //cut-off
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println(">>> after callback method");
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(DELAY_TIME);
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("\n[Jenkins REST Client]"));
  pixels.begin(); // This initializes the NeoPixel library.

  setColor(0,0,0);

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
    timer = millis() + REQUEST_TIMEOUT;
    Serial.println();
    Serial.print("<<< REQ ");
    Serial.println(F("Before browseUrl"));
    ether.browseUrl(PSTR("/"), "jenk2", website, my_callback);
    Serial.println(F("After browseUrl"));
  }
}
