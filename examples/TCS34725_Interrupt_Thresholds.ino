#include "TCS34725AutoGain.h"

TCS34725 tcs;

void setup() {
  Serial.begin(115200);
  Serial.println();

  Wire.begin();
  if (!tcs.attach()) {
    Serial.println("Error, no TCS detected");
    return;
  }

  tcs.autoGain();
  Serial.print("Autogain has selected gain x");
  Serial.print(tcs.gain());
  Serial.print(", integration time ");
  Serial.print(tcs.integrationTime());
  Serial.println("ms, beginning measurements:");

  // don't generate an interrupt on every completed read, but only for reads
  // whose clear count was outside the threshold range
  tcs.persistence(0x01);
}

void loop() {
  TCS34725::RawData raw = tcs.raw();
  // calculate +- 20% thresholds
  uint16_t low = raw.c * .8;
  uint16_t high = raw.c * 1.2;
  tcs.interruptThresholds(low, high);
  Serial.print("Raw clear count: ");
  Serial.print(raw.c);
  Serial.print(", waiting for interrupt indicating a measurement outside [");
  Serial.print(low);
  Serial.print(", ");
  Serial.print(high);
  Serial.println("].");

  // poll for next out-of-threshold read
  while (!tcs.available()) {
    Serial.print('.');
    delay(tcs.integrationTime());
  }
  Serial.println('!');
}
