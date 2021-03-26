//
//  HASTYtraffic
//
//  Description: QSO generator for testing
//
//  RELEASE: 20210326-0600
//
//
// Built specifically for the TTGO-T-BEAM
// To make it compile, install the esp32 boards in Arduino board manager from:
//    https://github.com/espressif/arduino-esp32
// The Lora libraries used in these examples are Arduino-lora, from:
//    https://github.com/sandeepmistry/arduino-LoRa
// Code is leveraged ttgo examples:
//    https://github.com/LilyGO/TTGO-T-Beam
//
// Transmits a rambling QSO. When it hears a station, acknowledges, gives a RSSI report, and then goes back to its QSO.
// See comments in program where you can add/change the QSO messages
//


//
// DEFINES
//

#include <SPI.h>
#include <LoRa.h>                          // https://github.com/sandeepmistry/arduino-LoRa

// SPI LoRa Radio
#define LORA_SCK 5        // GPIO5 - SX1276 SCK
#define LORA_MISO 19     // GPIO19 - SX1276 MISO
#define LORA_MOSI 27    // GPIO27 -  SX1276 MOSI
#define LORA_CS 18     // GPIO18 -   SX1276 CS
#define LORA_RST 14   // GPIO14 -    SX1276 RST
#define LORA_IRQ 26  // GPIO26 -     SX1276 IRQ (interrupt request)


//
// VARIABLES
//


String statA = "VIOLET-50";                // Station A
String statB = "PURPLE-50";                // Station B
String headAB = "VIOLET-50>PURPLE-50|";    // Station A to B header
String headBA = "PURPLE-50>VIOLET-50|";    // Station B to A header
String newby = "";                         // Station heard
bool takeTURNS = true;                     // Whose turn to talk toggle

String outgoing;                           // outgoing message
String payload;                            // payload in outgoing message

String ragcINT = "Wait a sec. We got a breaking station ...";
String ragcACK = "Your 599 599. Frequency is in use. 73";
String ragcRET = "OK. Where were we";

long lastSendTime = 0;                     // last send time
int interval = 2000;                       // interval between sends


//
// SETUP FUNCTIONS
//

void initSerial() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 12, 15);   //17-TX 18-RX for GPS
  Serial.println();
  Serial.println();
}

void initLoRa() {
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS); 
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);         
  if (!LoRa.begin(911250000)) {
    Serial.println("Starting LoRa failed!");
    while (1); // if LoRa won't start just die with infinite loop
  }
  LoRa.setSpreadingFactor(7); // ranges from 6-12, default 7 see API docs. Changed for ver 0.1 Glacierjay
  LoRa.setFrequency(911250000);
  LoRa.setCodingRate4(8);
  LoRa.setSignalBandwidth(125000);
  LoRa.setTxPower(14, PA_OUTPUT_PA_BOOST_PIN);
}

void setup()
{
  initSerial();
  Serial.println("INIT: HASTYtest_ttgo");
  Serial.println("=====================================");
  Serial.print(" 000: Setup");
  initLoRa();
}


//
// MAIN FuNCTIONS
//

void txMessage (String outbound) {
  LoRa.beginPacket();                      // start packet
  LoRa.print(outbound);                    // add payload
  LoRa.endPacket();                        // finish packet and send it
  Serial.println("TX:" + outbound);
}

void ragChew() {
  int idxnum = random(1,12);
  switch (idxnum) {
    //
    // Add your own ragchew messages as a case anbd remember to up the max for random above
    //
    case 1:
      payload = "Yeah well I told him he was overdeviating with that new fangled DMR radio";
      break;
    case 2:
      payload = "Right, but still it can't do CW. I mean how can it call itself a radio";
      break;
    case 3:
      payload = "of course I told the kid what are you going to do when the Internet is down";
      break;
    case 4:
      payload = "Well you know he got the Groats syndrome as well. I warned him Pepto Bismol has the opposite effect.";
      break;
    case 5:
      payload = "Thanks again for telling me about that special on D cell batteries at Harbor Freight";
      break;
    case 6:
      payload = "I'll be back in a few. Gotta make a fresh pot of coffee";
      break;
    case 7:
      payload = "Yeah, Internet access at the FD site. Our old Win7 laptops from last year and the network were slow";
      break;
    case 8:
      payload = "It was a cool looking go-kit with a Yaesu FT-101 and PS inside a gator box. But he always has issues with it";
      break;
    case 9:
      payload = "Fer sure good buddy, 57 repeaters and nothings on";
      break;
    case 10:
      payload = "Who was that recent SK?  Not the two from 3 weeks weeks ago or the 1 from last week";
      break;
    case 11:
      payload = "What was with that presentation at the last club meetingwhere all the guy did was read what was on the slides to us?";
      break;
    case 12:
      payload = "BTW I heard the ARRL NTS has been either getting spammed or unknowingly be used by foreign agents to send messages";
      break; 
    default:
      break;
  }
}

void breakerbreaker(String newby) {
  //
  // A new signal shows up. How dare they. Don't they know the frequency is busy 
  //
  outgoing = headAB + "Wait a sec. We got a breaking station ...";
  txMessage (outgoing);
  delay (4000);
  outgoing = statA + ">" + newby +"|" + "Your 599 599 and RSSI " + String(LoRa.packetRssi()) + ". Frequency is in use. 73";
  txMessage (outgoing);
  delay (4000);
  outgoing = headAB + "OK. Where were we";  // after that rude interruption
  txMessage (outgoing);
  delay (4000);
} 

void onReceive(int packetSize) {
  if (packetSize == 0) return;            // if there's no packet, return
  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if ((incoming.substring(0,19) != headAB) || (incoming.substring(0,19) != headBA)) {
    int delimiter = incoming.indexOf(">");
    newby = incoming.substring(0,delimiter);
    breakerbreaker(newby);
  }

  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}

void loop() {
  if (millis() - lastSendTime > interval) {
    ragChew();
    if (takeTURNS == false) {
      outgoing = headAB + payload;
      takeTURNS = true;
    }
    else {
      outgoing = headBA + payload;
      takeTURNS = false;
    }
    txMessage (outgoing);
    lastSendTime = millis();              // timestamp the message
    interval = random(3000) + 9000;       // 9-12 seconds
  }
  
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}
