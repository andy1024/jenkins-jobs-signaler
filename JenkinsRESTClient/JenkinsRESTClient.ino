// Arduino Jenkins REST client
//
// Copyright: GPL V2
// See http://www.gnu.org/licenses/gpl.html
// by https://github.com/andy1024
// based on EtherCard and NeoPixel examples

//pin layout:
//13- eth CLK/SCK
//12- eth SO
//11- eth SI/ST
//08- eth CS/SS
//09- r470->ws2811 dinput

#include <EtherCard.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

static byte mymac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };

#define MAX_BUF 400

byte Ethernet::buffer[MAX_BUF];
static uint32_t timer;

#define DELAY_TIME 50

#define REQUEST_TIMEOUT 30000

//NeoPixel definitions
#define PIN            9

//How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1

#define SUCCESS "SUCCESS"
#define FAILURE "FAILURE"
#define ABORTED "ABORTED"
#define UNSTABLE "UNSTABLE"

const char website[] PROGMEM = "192.168.1.21";
const char url_job[] PROGMEM = "/job/DailyBuild/";
const char rest_command[] PROGMEM = "lastCompletedBuild/api/json";//?tree=result";
const char result_pattern_mark[] PROGMEM = "\"result\":\"";
//ether.browseUrl(url_job, rest_command, website, my_callback);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// request callback
static void my_callback (byte status, word off, word len) {
  int jobStatus;
  Serial.println(F(">>> in callback method"));
  Ethernet::buffer[off+MAX_BUF] = 0; //cut-off
  //Serial.println((const char*) Ethernet::buffer + off);
  Serial.print(F("Result #"));
  jobStatus = get_status((const char*) Ethernet::buffer + off);
  setStatusColor(jobStatus);
  Serial.print(jobStatus);
  Serial.println(F("\n>>> after callback method"));
}

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(DELAY_TIME);
}

void setStatusColor(int status) {
  switch (status) {
    case 1: setColor(0,255,0); break;
    case 2: setColor(255,0,0); break;
    case 3: setColor(80,80,80); break;
    case 4: setColor(255,255,0); break;
    default: setColor(0,0,0); break;
  }
}

int get_status(const char* input) {
  char c;
  char buf[MAX_BUF];
  unsigned char buf_index = 0;
  unsigned char scan_index = 0;
  unsigned char retval = 0;

  unsigned char found = 0;
  int i=0;
  //Serial.print("patternlen=");
  //Serial.println(strlen(result_pattern_mark));
  //scan the input stream
  while (c=(*(input)++)) {
    //Serial.print("i=");
    //Serial.print(i++);
    //Serial.print(" c=");
    //Serial.println(c);
    //if pattern scanning positive
    if ((found == 0) && (scan_index >= strlen(result_pattern_mark))) {
      //data collecting state
      found = 1;
      buf_index = 0;
    }
    //if in collecting state
    if (found) {
      //check the boundaries, if so -> mark the result end
      if (c=='\"'||buf_index>=MAX_BUF) {
        buf[buf_index] = 0;
        break;
      } else {
        buf[buf_index++] = c;
      }
    } else { //if in scanning state
      //check if current char matches the pattern
      if (c == pgm_read_byte_near(result_pattern_mark + scan_index)) {
        ++scan_index;
      } else {
        //if it doesn't, reset the scanning index
        scan_index = 0;
      }
    }
  }
  
  //translate the results
  if (strlen(buf)>6) {
    if (strcmp(buf, SUCCESS)==0) retval = 1;
    if (strcmp(buf, FAILURE)==0) retval = 2;
    if (strcmp(buf, ABORTED)==0) retval = 3;
    if (strcmp(buf, UNSTABLE)==0) retval = 4;
  }
  //printf("result=%s\n", buf);
  return retval;
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("\n[Jenkins REST Client]"));
  pixels.begin(); // This initializes the NeoPixel library.

  setColor(0,0,0);

  /*
  Serial.print(F("MAC assigned: "));
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  */
  Serial.println(F("\nTrying to connect"));
  
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
    Serial.print(F("<<< REQ "));
    Serial.println(F("Before browseUrl"));
    ether.browseUrl(url_job, "lastCompletedBuild/api/json?tree=result", website, my_callback);
    Serial.println(F("After browseUrl"));
  }
}
