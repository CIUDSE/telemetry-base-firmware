#include <Arduino.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <ReactESP.h>
#include <axp20x.h>

// LoRa Chip (SX1278) SPI
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND 915E6 // Can also use 868 Mhz bands 868E6


// OLED (SSD1306) I2C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3c
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// GPS Serial1
TinyGPSPlus gps;
// HardwareSerial Serial1(1);
// Serial1 already defined as global in HardwareSerial.cpp

// Power Managment IC (AXP192)
// AXP20X_Class axp;

int global_heartbeat = 0;
int gps_serial_heartbeat = 0;
int lora_heartbeat = 0;
int latest_rssi = 0;
float latest_long = 0;
float latest_lat = 0;

void all_println(const char *msg) {
  Serial.println(msg);
  //display.println(msg);
  //display.display();
}

void fetch_gps_data() {
  while (Serial1.available()) {
    gps.encode(Serial1.read());
    gps_serial_heartbeat = (gps_serial_heartbeat + 1) % 10000;
  }
}

ReactESP app([] () {
  Serial.begin(115200);

  /*
  if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS)) {
    Serial.println("AXP192 Begin PASS");
  } else {
    Serial.println("AXP192 Begin FAIL");
  }
  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  */

  /*
  Serial.println("OLED...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 alloc err");
    for(;;); // Loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  all_println("OLED OK");
  */

  all_println("LoRa...");
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if(!LoRa.begin(BAND)) {
    Serial.println("LoRa init err");
    for(;;); // Loop forever
  }
  all_println("LoRa OK");

  all_println("GPS...");
  Serial1.begin(9600, SERIAL_8N1, 34, 12);
  app.onTick(fetch_gps_data);
  all_println("GPS OK");

  all_println("OK");
  
  app.onTick([] () {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      lora_heartbeat = (lora_heartbeat + 1) % 10000;

      while(LoRa.available()){
        Serial.print((char) LoRa.read());
      }
      Serial.print("RSSI ");
      latest_rssi = LoRa.packetRssi();
      Serial.println(latest_rssi);
    }
  });

  /*
  app.onRepeat(1000, [] () {
    display.clearDisplay();
    display.setCursor(0, 0);

    display.printf("GLB %i", global_heartbeat);
    display.printf("GPS %i\n", gps_serial_heartbeat);    
    display.printf("LORA %i\n", lora_heartbeat);
    display.printf("RSSI %i\n", latest_rssi);

    display.display();
    global_heartbeat = (global_heartbeat + 1) % 1000;
  });
  */
});