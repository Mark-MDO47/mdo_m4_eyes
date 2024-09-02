// Skull Project - https://github.com/Mark-MDO47/Skull-Project
// Author:  https://github.com/Mark-MDO47
// Date:    2024-09-01
// License: MIT
//
#if 1 // enables this user file

#include "globals.h"

#define PIR_PIN A8           // (D2) PIR sensor on the "sensor" connector of Hallowing M4
#define ALWAYS_DSPLY_PIN 5   // output HIGH to LED if ALWAYS-ON
#define ALWAYS_SENSE_PIN 6   // input LOW if ALWAYS-ON

static uint32_t millisec_for_off;

//-----------------------------------------------------------------------------
//
// user_setup() - user-provided code for the setup
//
// Called once near the end of the setup() function. If your code requires
// a lot of time to initialize, make periodic calls to yield() to keep the
// USB mass storage filesystem alive.
//
// This code sets the pinMode for our pins and then puts the screen timeout in an initial state
//
void user_setup(void) {
  pinMode(PIR_PIN, INPUT_PULLUP);
  pinMode(ALWAYS_DSPLY_PIN, INPUT_PULLUP);
  pinMode(ALWAYS_SENSE_PIN, OUTPUT);
  millisec_for_off = 10000 + millis(); // we start with display on; this is TRUE first time through loop()
}  // end user_setup()

//-----------------------------------------------------------------------------
//
// user_loop() - user-provided code for the loop
//
// Called periodically during eye animation. This is invoked in the
// interval before starting drawing on the last eye (left eye on MONSTER
// M4SK, sole eye on HalloWing M0) so it won't exacerbate visible tearing
// in eye rendering. This is also SPI "quiet time" on the MONSTER M4SK so
// it's OK to do I2C or other communication across the bridge.
// This function BLOCKS, it does NOT multitask with the eye animation code,
// and performance here will have a direct impact on overall refresh rates,
// so keep it simple. Avoid loops (e.g. if animating something like a servo
// or NeoPixels in response to some trigger) and instead rely on state
// machines or similar. Additionally, calls to this function are NOT time-
// constant -- eye rendering time can vary frame to frame, so animation or
// other over-time operations won't look very good using simple +/-
// increments, it's better to use millis() or micros() and work
// algebraically with elapsed times instead.
//
// This code sets checks our two input pins and the screen timeout
//    It sets the output pin to light the button LED to show our processing state
//    It determines if the screen timeout should be extended and does so if needed
//    It checks if the screen timeout expired and turns the screen backlight off if needed
//
// This code is designed to be easy to read, not fast. It is fast enough.
//
void user_loop(void) {
  uint32_t now = millis();
  uint16_t always_sense = (LOW == digitalRead(ALWAYS_SENSE_PIN));

  // set button LED to show state of ALWAYS-ON
  if (always_sense) {
    digitalWrite(ALWAYS_DSPLY_PIN, HIGH);
  } else {
    digitalWrite(ALWAYS_DSPLY_PIN, LOW);
  }

  // determine if we should extend  millisec_for_off
  if (always_sense) {
    millisec_for_off = 500 + now; // rapid refresh so respond quickly if user turns button off
  } else if (HIGH == digitalRead(PIR_PIN)) {
    millisec_for_off = 20000 + now; // PIR detect causes long timeout
  }

  // if later than millisec_for_off, turn backlight off otherwise turn it on
  // NOTE: it will be more than 49 days before we overflow the millisec counts
  //   battery would be discharged long before that
  // Will need to get more complicated if powering with wall power.
  if (now > millisec_for_off) {
    arcada.setBacklight(0); // turn screen off
  } else {
    arcada.setBacklight(255); // turn screen on
  }
  
}  // end user_loop()

#endif // enable/disable this user file