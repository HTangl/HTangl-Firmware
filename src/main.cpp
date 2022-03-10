/*
  HayB0XX Version 1.0.0

  Some parts of this code were originally based on GCCPCB2 v1.208 code by Crane.

  This code utilizes
    Nicohood's Nintendo library
    MHeironimus' Arduino Joystick Library
    Arduino Keyboard Library
    Zac Staples' Arduino_Vector library
*/

#define NUNCHUK_ENABLE true

#include <Arduino.h>

#if NUNCHUK_ENABLE
#include <NintendoExtensionCtrl.h>
#endif

#include "config/setup.h"
#include "config/mode_selection.h"
#include "core/InputMode.h"

enum reportState : byte {
  ReportOff = 0x30,
  ReportOn = 0x31,
  ReportEnd = 0x0A,
  ReportInvalid = 0x00
};

byte report[25] = {};

// Used to count updates so we only send state to the input viewer every 3
// updates.
int gReportClock = 0;

void writeSerialReport();

extern InputMode *gCurrentMode;
extern state::InputState gInputState;

#if NUNCHUK_ENABLE
Nunchuk gNunchuk;
#endif

void readInputs() {
  // B0XX inputs.
  gInputState.l = (digitalRead(pinout::L) == LOW);
  gInputState.left = (digitalRead(pinout::LEFT) == LOW);
  gInputState.down = (digitalRead(pinout::DOWN) == LOW);
  gInputState.right = (digitalRead(pinout::RIGHT) == LOW);
  gInputState.mod_x = (digitalRead(pinout::MODX) == LOW);
  gInputState.mod_y = (digitalRead(pinout::MODY) == LOW);
  gInputState.start = (digitalRead(pinout::START) == LOW);
  gInputState.select = (digitalRead(pinout::SELECT) == LOW);
  gInputState.home = (digitalRead(pinout::HOME) == LOW);
  gInputState.b = (digitalRead(pinout::B) == LOW);
  gInputState.x = (digitalRead(pinout::X) == LOW);
  gInputState.z = (digitalRead(pinout::Z) == LOW);
  gInputState.up = (digitalRead(pinout::UP) == LOW);
  gInputState.r = (digitalRead(pinout::R) == LOW);
  gInputState.y = (digitalRead(pinout::Y) == LOW);
  gInputState.lightshield = (digitalRead(pinout::LIGHTSHIELD) == LOW);
  gInputState.midshield = (digitalRead(pinout::MIDSHIELD) == LOW);
  gInputState.c_down = (digitalRead(pinout::CDOWN) == LOW);
  gInputState.a = (digitalRead(pinout::A) == LOW);
  gInputState.c_right = (digitalRead(pinout::CRIGHT) == LOW);
  gInputState.c_left = (digitalRead(pinout::CLEFT) == LOW);
  gInputState.c_up = (digitalRead(pinout::CUP) == LOW);

// Nunchuk inputs
#if NUNCHUK_ENABLE  
  if (gInputState.nunchuk_connected && gNunchuk.update()) {
    gInputState.nunchuk_x = gNunchuk.joyX();
    gInputState.nunchuk_y = gNunchuk.joyY();
    gInputState.nunchuk_c = gNunchuk.buttonC();
    gInputState.nunchuk_z = gNunchuk.buttonZ();
  }
#endif
}

void setup() {
#if NUNCHUK_ENABLE
delay(50);
    gNunchuk.begin();
    if (gNunchuk.connect()) {
      gInputState.nunchuk_connected = true;
    } else {
      gNunchuk.i2c().end();
    }
#endif

  pinMode(pinout::L, INPUT_PULLUP);
  pinMode(pinout::LEFT, INPUT_PULLUP);
  pinMode(pinout::DOWN, INPUT_PULLUP);
  pinMode(pinout::RIGHT, INPUT_PULLUP);
  pinMode(pinout::MODX, INPUT_PULLUP);
  pinMode(pinout::MODY, INPUT_PULLUP);
  pinMode(pinout::START, INPUT_PULLUP);
  pinMode(pinout::SELECT, INPUT_PULLUP);
  pinMode(pinout::HOME, INPUT_PULLUP);
  pinMode(pinout::B, INPUT_PULLUP);
  pinMode(pinout::X, INPUT_PULLUP);
  pinMode(pinout::Z, INPUT_PULLUP);
  pinMode(pinout::UP, INPUT_PULLUP);
  pinMode(pinout::R, INPUT_PULLUP);
  pinMode(pinout::Y, INPUT_PULLUP);
  pinMode(pinout::CDOWN, INPUT_PULLUP);
  pinMode(pinout::A, INPUT_PULLUP);
  pinMode(pinout::CRIGHT, INPUT_PULLUP);
  pinMode(pinout::CLEFT, INPUT_PULLUP);
  pinMode(pinout::CUP, INPUT_PULLUP);
  pinMode(pinout::LIGHTSHIELD, INPUT_PULLUP);
  pinMode(pinout::MIDSHIELD, INPUT_PULLUP);

  // Read initial inputs into gInputState to make backend selection logic in
  // initialise() a bit cleaner.
  readInputs();

  // Controller-specific setup.
  initialise();
}

void loop() {
  readInputs();

  /* Mode selection */
  selectInputMode();

  // Only run input viewer on every 3 updates, to prevent lag.
  if (Serial.availableForWrite() > 32) {
    if (gReportClock == 0) {
      writeSerialReport();
      gReportClock++;
    } else if (gReportClock == 3) {
      gReportClock = 0;
    } else {
      gReportClock++;
    }
  }

  gCurrentMode->UpdateOutputs();
}

/**
 * Write serial report for B0XX input viewer.
 */
void writeSerialReport() {
  report[0] = gInputState.start ? ReportOn : ReportOff;
  report[1] = gInputState.y ? ReportOn : ReportOff;
  report[2] = gInputState.x ? ReportOn : ReportOff;
  report[3] = gInputState.b ? ReportOn : ReportOff;
  report[4] = gInputState.a ? ReportOn : ReportOff;
  report[5] = gInputState.l ? ReportOn : ReportOff;
  report[6] = gInputState.r ? ReportOn : ReportOff;
  report[7] = gInputState.z ? ReportOn : ReportOff;
  report[8] = gInputState.up ? ReportOn : ReportOff;
  report[9] = gInputState.down ? ReportOn : ReportOff;
  report[10] = gInputState.right ? ReportOn : ReportOff;
  report[11] = gInputState.left ? ReportOn : ReportOff;
  report[12] = gInputState.mod_x ? ReportOn : ReportOff;
  report[13] = gInputState.mod_y ? ReportOn : ReportOff;
  report[14] = gInputState.c_left ? ReportOn : ReportOff;
  report[15] = gInputState.c_right ? ReportOn : ReportOff;
  report[16] = gInputState.c_up ? ReportOn : ReportOff;
  report[17] = gInputState.c_down ? ReportOn : ReportOff;
  report[18] = gInputState.lightshield ? ReportOn : ReportOff;
  report[19] = gInputState.midshield ? ReportOn : ReportOff;
  report[20] = ReportOff;
  report[21] = ReportOff;
  report[22] = ReportOff;
  report[23] = ReportOff;
  report[24] = ReportEnd;

  Serial.write(report, 25);
}
