#include <Adafruit_NeoPixel.h>

#define PIN_LED 6
#define NUM_LEDS 20

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    
    if (msg == "teste") {
      Serial.println("OK");
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 0, 0));
      }
      strip.show();
      delay(1000);
      strip.clear();
      strip.show();
    }
  }
}
