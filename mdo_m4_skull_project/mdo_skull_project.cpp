// Skull Project - https://github.com/Mark-MDO47/Skull-Project
// Author:  https://github.com/Mark-MDO47
// Date:    2024-09-01
// License: MIT
//
// This code runs unchanged in both primary eye and secondary (2nd) eye;
//        pin SCNDEYE_1ST_EYE_PIN tells us which one we are.
//
// Primary eye: code checks our FORCE_ON and MOTION_SENSOR input pins and the sets screen timeout
//    Primary eye has no connect for SCNDEYE_1ST_EYE_PIN so it reads HIGH (due to internal pullup)
//    It determines if the screen timeout should be extended and does so if needed
//    It checks if the screen timeout expired and turns the screen backlight off if needed
//      It copies the calculated display on/off value to the secondary Hallowing using SCNDEYE_DSPLY_ON_PIN
//    It sets the DISPLAY_FORCE_ON_LED_PIN output pin to light the button LED to show our ALWAYS-ON processing state
// Secondary eye: it merely uses on/off from primary SCNDEYE_DSPLY_ON_PIN to turn secondary display on/off.
//    Secondary eye has grounded SCNDEYE_1ST_EYE_PIN so it reads LOW
//    Ssecondary eye reads DISPLAY_FORCE_ON_PIN (from primary SCNDEYE_DSPLY_ON_PIN) to turn display on or off
//
// This code is designed to be easy to read, not fast as possible. It is fast enough.
//

#if 1 // enables this user file

#include "globals.h"

#define MOTION_SENSOR_PIN         A8   // (D2) PIR sensor on the "sensor" connector of Hallowing M4
#define DISPLAY_FORCE_ON_LED_PIN   5   // output HIGH to LED if backlight forced ALWAYS-ON
#define DISPLAY_FORCE_ON_PIN       6   // input primary eye: LOW if backlight forced ALWAYS-ON
                                       // input secondary eye: HIGH turns backlight ON, LOW turns OFF
#define SCNDEYE_DSPLY_ON_PIN       9   // mirror Display On to other Hallowing if we are primary Hallowing
#define SCNDEYE_1ST_EYE_PIN       10   // HIGH if primary Hallowing; LOW if secondary Hallowing
// NOTE:
//   SCNDEYE_DSPLY_ON_PIN unused on 2nd eye - we use this fact in setup()
//   DISPLAY_FORCE_ON_LED_PIN unused on 2nd eye - this fact is not used
//   MOTION_SENSOR_PIN unused on 2nd eye - this fact is not used
// NOTE:
//   DISPLAY_FORCE_ON_PIN used differently on primary and secondary eye
//      primary: used to sense state of button that sets ALWAYS-ON on or off
//      secondary: used to sense output from primary from SCNDEYE_DSPLY_ON_PIN

static uint32_t millisec_for_off;

#define MSEC_ON_4_PIR 20000   // number of milliseconds to stay on past last PIR HIGH reading
                              // this value chosen so there can be interaction and surprise with on/off
#define MSEC_ON_4_ALWAYS 500  // number of milliseconds to stay on past last button forcing ALWAYS-ON
                              // this value chosen so there is fairly fast reaction to button change

#define DEBUG_DSPLY_PIN 0   // set to 1 to blink the button LED on and off
#define DEBUG_SENSE_PIN 0   // set to 1 to print button changes on serial port
                            // NOTE: Serial.begin() called in *.ino setup() routine after calling us


//-----------------------------------------------------------------------------
//
// dbg_dsply_pin() - blink the button LED on and off
//
#if DEBUG_DSPLY_PIN
void dbg_dsply_pin(uint32_t now) {
  static uint32_t dbg_dsply_pin_timeout = 0;
  static int value = HIGH;
  if (now > dbg_dsply_pin_timeout) {
    if (value == HIGH) value = LOW;
    else               value = HIGH;

    if (value == HIGH) Serial.println("HIGH");
    else               Serial.println("LOW");
    dbg_dsply_pin_timeout = 2000 + now;
  }
  digitalWrite(DISPLAY_FORCE_ON_LED_PIN, value);
}  // end dbg_dsply_pin()
#endif // DEBUG_DSPLY_PIN

//-----------------------------------------------------------------------------
//
// dbg_sense_pin() - tell when forcing ALWAYS-ON state changes
//
#if DEBUG_SENSE_PIN
#endif // DEBUG_SENSE_PIN
void dbg_sense_pin(uint16_t always_sense_or_2nd_display_off) {
  static int16_t always_sense_or_2nd_display_off_prev = -1;
  if ((-1 == always_sense_or_2nd_display_off_prev) || (always_sense_or_2nd_display_off != always_sense_or_2nd_display_off_prev)) {
    always_sense_or_2nd_display_off_prev = always_sense_or_2nd_display_off;
    if (always_sense_or_2nd_display_off) {
      Serial.println("TRUE == ALWAYS_ON");
    } else {
      Serial.println("FALSE == ALWAYS_ON");
    }
  }
} // end dbg_sense_pin()

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
  pinMode(MOTION_SENSOR_PIN, INPUT_PULLUP);    // input HIGH momentarily when detect person; PIR sensor (D2)
  pinMode(DISPLAY_FORCE_ON_PIN, INPUT_PULLUP); // input LOW if FORCE ALWAYS-ON
  pinMode(DISPLAY_FORCE_ON_LED_PIN, OUTPUT);   // output HIGH to LED if FORCE ALWAYS-ON
  pinMode(SCNDEYE_DSPLY_ON_PIN, OUTPUT);       // output mirror Display On to other Hallowing if we are primary Hallowing
  pinMode(SCNDEYE_1ST_EYE_PIN, INPUT_PULLUP);  // input HIGH if primary Hallowing; LOW if secondary Hallowing
  
  millisec_for_off = MSEC_ON_4_PIR + millis(); // we start with display on; this is TRUE first time through loop()
  digitalWrite(SCNDEYE_DSPLY_ON_PIN, HIGH);    //   not sure how long the rest of setup will take so start HIGH
                                               //   SCNDEYE_DSPLY_ON_PIN unused on 2nd eye
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
// https://github.com/Mark-MDO47
// This code sets checks our two input pins and the screen timeout
//    It determines if the screen timeout should be extended and does so if needed
//    It checks if the screen timeout expired and turns the screen backlight off if needed
//    It sets the DISPLAY_FORCE_ON_LED_PIN output pin to make the button LED show our FORCE ALWAYS-ON processing state
//    It copies the calculated display on/off value to the secondary Hallowing
//        or (if secondary) uses on/off from primary to turn secondary display on/off.
// This code is designed to be easy to read, not fast as possible. It is fast enough.
//
// Optional 2nd Eye accomodation uses pins SCNDEYE_1ST_EYE_PIN (input) and SCNDEYE_DSPLY_ON_PIN (output)
//    primary eye has no connect for SCNDEYE_1ST_EYE_PIN so it reads HIGH (due to internal pullup)
//    2nd eye connects SCNDEYE_1ST_EYE_PIN to ground so it reads LOW
//
//    primary eye calculates display on/off based on PIR and DISPLAY_FORCE_ON_PIN; sends to SCNDEYE_DSPLY_ON_PIN
//    secondary eye reads display on/off from DISPLAY_FORCE_ON_PIN (connected to primary SCNDEYE_DSPLY_ON_PIN)
//
void user_loop(void) {
  uint32_t now = millis();
  uint16_t always_sense_or_2nd_display_off = (LOW == digitalRead(DISPLAY_FORCE_ON_PIN));
  uint8_t  pir_value = digitalRead(MOTION_SENSOR_PIN); // input is HIGH ~2 sec. when detect person; PIR sensor (D2)


#if DEBUG_SENSE_PIN
  dbg_sense_pin(always_sense_or_2nd_display_off);
#endif // DEBUG_SENSE_PIN

#if DEBUG_DSPLY_PIN
  dbg_dsply_pin(now); // blink the button LED
#else // not DEBUG_DSPLY_PIN
  // set button LED to show state of FORCE ALWAYS-ON
  if (always_sense_or_2nd_display_off) {
    digitalWrite(DISPLAY_FORCE_ON_LED_PIN, HIGH);
  } else {
    digitalWrite(DISPLAY_FORCE_ON_LED_PIN, LOW);
  }
#endif // not DEBUG_DSPLY_PIN

  // determine if we should extend  millisec_for_off
  if (always_sense_or_2nd_display_off) {
    millisec_for_off = MSEC_ON_4_ALWAYS + now; // rapid refresh so respond quickly if user turns button off
  } else if (HIGH == pir_value) {
    millisec_for_off = MSEC_ON_4_PIR + now; // PIR detect causes long timeout
  }

  // if later than millisec_for_off, turn backlight off otherwise turn it on
  // NOTE: it will be more than 49 days before we overflow the millisec counts;
  //   battery would be discharged long before that
  // TODO - Will need to handle wraparound if powering with wall power.
  //
  // if two Hallowing M4 and we are the primary one connected to PIR sensor
  //    mirror the display onvalue to the other one
  //
  if (HIGH == digitalRead(SCNDEYE_1ST_EYE_PIN)) {
    // we are primary Hallowing; use timer and notify other side
    if (now > millisec_for_off) {
      arcada.setBacklight(0); // turn screen off
      digitalWrite(SCNDEYE_DSPLY_ON_PIN, LOW);
    } else {
      arcada.setBacklight(255); // turn screen on
      digitalWrite(SCNDEYE_DSPLY_ON_PIN, HIGH);
    }
  } else {
    // we are 2nd Hallowing; follow directions from primary
    if (always_sense_or_2nd_display_off) {
      arcada.setBacklight(0); // turn screen off
    } else {
      arcada.setBacklight(255); // turn screen on
    }
  } // end check if primary or secondary eye
}  // end user_loop()

#endif // enable/disable this user file
