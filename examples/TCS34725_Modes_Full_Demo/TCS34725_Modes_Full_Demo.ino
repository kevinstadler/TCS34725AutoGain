#include "TCS34725AutoGain.h"

TCS34725 tcs;

#define NREADS 3
void doBusyReads(byte nReads = NREADS);
void doSingleReads(byte nReads = NREADS);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  Wire.begin();
  if (!tcs.attach(Wire, TCS34725::Mode::Undefined)) {
    Serial.println("Error, no TCS detected");
    return;
  }

  Serial.println("According to the datasheet, the reset value of the enable register is supposed to be 0x00,");
  Serial.println("i.e. on power-on the chip should be in sleep mode. In my experience the enable register");
  Serial.println("is actually reset to 0x1b, i.e. AIEN, PON, AEN, WEN are all set to on, meaning the chip will");
  Serial.println("be in a read - wait - read cycle, and raise interrupts when new readings arrive.");
  Serial.println();
  Serial.println("Other registers like ATIME and WTIME show significant persistence even after long power cuts,");
  Serial.println("so these will most likely have whatever values that were last written to them.");
  Serial.println();

  Serial.println("tcs.attach(Wire, TCS34725::Mode::Undefined) connects the sensor without touching/altering the enable register:");
  printTCSStatus();

  Serial.println();
  Serial.println("the default tcs.attach() with no arguments puts the sensor in a well-defined busy-reading mode:");
  tcs.attach();
  printTCSStatus();

  Serial.println();
  Serial.println("Setting x1 gain and 24ms integration time.");
  tcs.integrationCycles(10);
  tcs.gain(TCS34725::Gain::X01);
  printTCSStatus();

  Serial.println();
  Serial.println("Reading consecutive values by polling tcs.available()");
  doBusyReads();

  Serial.println();
  Serial.println("Changing gain to 4x. The first read after a parameter change might still be based on the previous gain and integration time");
  tcs.gain(TCS34725::Gain::X04);
  printTCSStatus();
  doBusyReads();

  Serial.println();
  Serial.println("Turning off busy reading");
  tcs.mode(TCS34725::Mode::Idle);
  printTCSStatus();
  Serial.println("Reading consecutive values through tcs.singleRead()");
  doSingleReads();
  printTCSStatus();

  Serial.println();
  Serial.print("Calling tcs.autoGain(2000) for a target clear count of 2000: ");
  Serial.println(tcs.autoGain(2000) ? "success" : "failed!");
  printTCSStatus();
  Serial.print("After autoGain(), there is a fresh set of data based on the final gain+integration time settings: ");
  printRaw(tcs.raw());

  Serial.println();
  Serial.println("Re-enabling busy reading and getting consecutive values through tcs.available()");
  tcs.mode(TCS34725::Mode::RGBC);
  printTCSStatus();
  doBusyReads();

  Serial.println();
  Serial.println("Going to sleep");
  tcs.mode(TCS34725::Mode::Sleep);
  printTCSStatus();

  Serial.println();
  Serial.print("Setting integration time to 2.4ms, duration from power on until first read:");
  tcs.integrationCycles(1);
  uint32_t m = micros();
  tcs.mode(TCS34725::Mode::RGBC);
  while (!tcs.available()) {
    delayMicroseconds(100);
  }
  m = micros() - m;
  Serial.print(m / 1000.0);
  Serial.println("ms");

  Serial.println();
  Serial.println("Setting a wait time of 200ms between automatic reads");
  tcs.wait(200);
  printTCSStatus();

  Serial.println();
  Serial.println("Entering read-wait-read mode and reading consecutive values by polling tcs.available()");
  tcs.mode(TCS34725::Mode::WaitRGBC);
  printTCSStatus();
  doBusyReads();

  Serial.println();
  Serial.println("Going to idle");
  tcs.mode(TCS34725::Mode::Idle);
  printTCSStatus();

  Serial.println();
  Serial.println("Finished.");
}

void printRaw(TCS34725::RawData raw) {
  Serial.print(raw.c);
  Serial.print('/');
  Serial.print(raw.r);
  Serial.print('/');
  Serial.print(raw.g);
  Serial.print('/');
  Serial.println(raw.b);
}

void doBusyReads(uint8_t nReads) {
  uint32_t m = micros();
  for (byte i = 1; i <= nReads; i++) {
    while(!tcs.available()) {
      delayMicroseconds(100);
    }
    Serial.print("Reading #");
    Serial.print(i);
    Serial.print(": after ");
    Serial.print((micros() - m) / 1000.0);
    Serial.print("ms got values ");
    printRaw(tcs.raw());
    m = micros();
  }
}

void doSingleReads(uint8_t nReads) {
  uint32_t m = micros();
  for (byte i = 1; i <= nReads; i++) {
    tcs.singleRead();
    Serial.print("Reading #");
    Serial.print(i);
    Serial.print(": after ");
    Serial.print((micros() - m) / 1000.0);
    Serial.print("ms got values ");
    printRaw(tcs.raw());
    m = micros();
  }
}

const String MODENAMES[5] = { "Undefined", "Sleep", "Idle", "RGBC", "RGBC+Wait" };

void printTCSStatus() {
  TCS34725::Mode m = tcs.mode();
  Serial.print("Mode ");
  Serial.print(MODENAMES[(uint8_t) m]);
  Serial.print(", gain x");
  Serial.print(tcs.gain());
  Serial.print(", integration time ");
  Serial.print(tcs.integrationTime());
  Serial.print("ms (");
  Serial.print(tcs.integrationCycles());
  Serial.print(" cycles), interrupts ");
  Serial.print(tcs.interrupt() ? "enabled" : "disabled");
  if (tcs.enable() & (uint8_t) TCS34725::Mask::ENABLE_WEN) {
    Serial.print(", WEN set with wait time of ");
    Serial.print(tcs.wait());
    Serial.print("ms");
  }
  Serial.println();
}

void loop() {
  delay(10000);
}

