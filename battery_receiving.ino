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

struct BatteryStatus {
  byte percentage;
};

Signal data;
BatteryStatus currentBattery = {0};  // Initialize battery percentage to 0
unsigned long prevTransmitTime = 0;
const unsigned int transmitInterval = 10; // 20ms interval (50Hz)
bool connected=false;

void setup() {
  if(radio.begin()){
    connected=true;
  }
  radio.openWritingPipe(pipeOut);
  radio.setCRCLength(RF24_CRC_16);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(100);
  radio.setRetries(5, 15);
  radio.setAutoAck(true);      // Enable auto-acknowledgment
  radio.enableAckPayload();    // Allow sending payloads with ACK
  radio.enableDynamicPayloads(); // Required for ACK payloads
  u8g2.begin();
}

void loop() {
  if (millis() - prevTransmitTime >= transmitInterval) {
    prevTransmitTime = millis();

    // Read control inputs
    data.throttle = analogRead(A1) / 4;
    data.direction = analogRead(A6) / 4;
    data.toggle = digitalRead(2);
    data.auxilary = analogRead(A3) / 4;

    // Transmit data and check for success
    bool success = radio.write(&data, sizeof(Signal));

    // Check if ACK payload (battery data) is received
    if (success && radio.available()) {
      radio.read(&currentBattery, sizeof(currentBattery));
      connected=true;
    }
    else{
      connected=false;
    }

    u8g2.clearBuffer();

    // === Battery Parameters ===
    int batteryX = 110;   // right edge
    int batteryY = 10;
    int batteryWidth = 12;
    int batteryHeight = 40;

    int fillMargin = 2;
    int fillHeight = map(currentBattery.percentage, 0, 100, 0, batteryHeight - (fillMargin * 2));

    // === Battery Outline and Fill ===
    u8g2.drawFrame(batteryX, batteryY, batteryWidth, batteryHeight); // battery shell
    u8g2.drawBox(batteryX + 3, batteryY - 3, 6, 3);                   // battery terminal head

    u8g2.drawBox(
      batteryX + fillMargin,
      batteryY + batteryHeight - fillMargin - fillHeight,
      batteryWidth - (fillMargin * 2),
      fillHeight
    );

    // === "GRIP" vertically beside battery ===
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setFontDirection(1); // rotate text 90 degrees clockwise
    u8g2.drawStr(batteryX - 6, batteryY + 10, "GRIP");
    u8g2.setFontDirection(0); // reset direction to normal

    // === Battery Percentage Text Below ===
    char battStr[10];
    sprintf(battStr, "%d%%", currentBattery.percentage);
    u8g2.setFont(u8g2_font_6x10_tr);
    int textWidth = u8g2.getStrWidth(battStr);
    u8g2.drawStr(batteryX + (batteryWidth / 2) - (textWidth / 2), batteryY + batteryHeight + 12, battStr);

    // === Connection Status Indicator ===
    if (connected) {
      u8g2.drawDisc(5, 10, 3); // filled circle = connected
    } else {
      u8g2.drawCircle(5, 10, 3); // hollow circle = not connected
    }
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(10, 13, "RF");

    // === Alert Symbol if Battery Low ===
    if (currentBattery.percentage < 20) {
      u8g2.setFont(u8g2_font_open_iconic_embedded_1x_t);
      u8g2.drawGlyph(10, 30, 0x0041); // ⚠️ triangle exclamation icon
      u8g2.setFont(u8g2_font_5x8_tr);
      u8g2.drawStr(20, 30, "Low Battery!");
    }

    u8g2.sendBuffer();

  }
}