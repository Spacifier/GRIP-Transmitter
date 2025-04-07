#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const uint64_t pipeOut = 0xE8E8F0F0E1;
RF24 radio(9, 10);

struct Signal {
  byte throttle;
  byte auxilary;  
  byte direction;
  byte toggle;
};

Signal data;
unsigned long prevTransmitTime = 0;
const unsigned int transmitInterval = 20; // 20ms interval (50Hz)

void setup() {
  //Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.setRetries(5, 15);
  radio.setAutoAck(false);
  u8g2.begin();
}

void loop() {

  if (millis() - prevTransmitTime >= transmitInterval) {
    prevTransmitTime = millis();

    // Read inputs
    data.throttle = analogRead(A1)/4;
    data.direction = analogRead(A6) / 4;
    data.toggle = digitalRead(2);
    data.auxilary = analogRead(A3) / 4;

    // Transmit data
    bool success = radio.write(&data, sizeof(Signal));
    // if(success){
    //   Serial.print("Throttle:"); 
    //   Serial.print(data.throttle);
    //   Serial.print(" | ");
    //   Serial.print("Direction:"); 
    //   Serial.print(data.direction);
    //   Serial.print(" | ");
    //   Serial.print("Auxilary:"); 
    //   Serial.print(data.auxilary);
    //   Serial.print(" | ");
    //   Serial.print("Toggle:"); 
    //   Serial.println(data.toggle);

    // }
    // else{
    //   Serial.println("Data not transmitted");
    // }

    //display
    u8g2.clearBuffer();

    // Draw battery outer shell
    u8g2.drawFrame(30, 20, 70, 30);  // x, y, w, h
    u8g2.drawBox(100, 27, 5, 15);    // battery head

    // Draw battery level (fill)
    u8g2.drawBox(32, 22, 50, 26);    // battery level bar

    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(35, 15, "Battery");

    u8g2.sendBuffer();

  }
}