// Skull Project - https://github.com/Mark-MDO47/Skull-Project
// This code -     https://github.com/Mark-MDO47/mdo_m4_eyes.git
//                   directory mdo_m4_skull_project
// Author:  https://github.com/Mark-MDO47
// Date:    2024-09-01
// License: MIT
//
// This code runs unchanged in both primary eye and secondary (2nd) eye;
//        pin SCNDEYE_1ST_EYE_PIN tells us which one we are.
//
// Primary eye: code checks our FORCE_ON and MOTION_SENSOR input pins and the sets screen timeout
//    Primary eye has no connect for SCNDEYE_1ST_EYE_PIN so it reads HIGH (due to internal pullup)
//    It sets the DISPLAY_FORCE_ON_LED_PIN output pin to light the button LED to show our ALWAYS-ON processing state
//    It determines if the screen timeout should be extended and does so if needed
//    It checks if the screen timeout expired and turns the screen backlight off if needed
//      It copies the calculated display on/off value to the secondary HalloWing using SCNDEYE_DSPLY_ON_PIN
// Secondary eye: it uses on/off from primary SCNDEYE_DSPLY_ON_PIN to turn secondary display on/off and does
//      a reset of primary eye a fixed time after bootup to avoid primary eye not booting properly.
//    Secondary eye has grounded SCNDEYE_1ST_EYE_PIN so it reads LOW
//    Secondary eye reads DISPLAY_FORCE_ON_PIN (from primary SCNDEYE_DSPLY_ON_PIN) to turn display on or off
//    Secondary eye outputs DISPLAY_FORCE_ON_LED_PIN to reset primary eye some time after reset
//
// This code is designed to be easy to read, not fast as possible. It is fast enough.
//    Admittedly the debug code makes it a little messy, but it is useful when things don't work.
//    Okay, all the differences between primary and secondary eye are really making it messy. Sorry.
//        Because of this I am removing the debug code. If you still want to see the debug code, look in
//        https://github.com/Mark-MDO47/mdo_m4_eyes.git
//        SHA ID 43f5a7991704ac46a373ff689222126ea42f681a
//
// If you want to read about problems with primary eye bootup see
//    https://github.com/Mark-MDO47/Skull-Project/blob/master/README.md#surprise---two-eye-version-usually-powers-up-wrong
//

#if 1 // enables this user file

#include "globals.h"

#define MOTION_SENSOR_PIN         A8   // (D2) PIR sensor on the "sensor" connector of HalloWing M4
#define DISPLAY_FORCE_ON_LED_PIN   5   // output HIGH to LED if backlight forced ALWAYS-ON
#define DISPLAY_FORCE_ON_PIN       6   // input primary eye: LOW if backlight forced ALWAYS-ON
                                       // input secondary eye: HIGH turns backlight ON, LOW turns OFF
#define SCNDEYE_DSPLY_ON_PIN       9   // mirror Display On to other HalloWing if we are primary HalloWing
#define SCNDEYE_1ST_EYE_PIN       10   // HIGH if primary HalloWing; LOW if secondary HalloWing
// NOTE:
//   SCNDEYE_DSPLY_ON_PIN unused on 2nd eye - we use this fact in setup()
//   MOTION_SENSOR_PIN unused on 2nd eye - this fact is not used
// NOTE:
//   DISPLAY_FORCE_ON_PIN used differently on primary and secondary eye
//      primary: used to sense state of button that sets ALWAYS-ON on or off
//      secondary: used to sense output from primary from SCNDEYE_DSPLY_ON_PIN
//   DISPLAY_FORCE_ON_LED_PIN  used differently on primary and secondary eye
//      primary: used to power LED of button if ALWAYS-ON is on
//      secondary: used to reset primary some time after power-on
//         to solve power-on-surge causing primary eye to boot incorrectly

static uint32_t millisec_for_off;          // primary - when to turn display off if no motion detected
static uint32_t millisec_for_wait_2_reset; // secondary - when to assert reset to primary
static uint32_t millisec_for_reset_off;    // secondary - when to de-assert reset to primary

#define MSEC_ON_4_PIR 20000    // number of milliseconds to stay on past last PIR HIGH reading
                               // this value chosen so there can be interaction and surprise with on/off
#define MSEC_ON_4_ALWAYS 500   // number of milliseconds to stay on past last button forcing ALWAYS-ON
                               // this value chosen so there is fairly fast reaction to button change

#define MSEC_ON_4_RESET 300    // number of milliseconds that secondary eye asserts reset to primary eye
#define MSEC_WAIT_4_RESET 7000 // number of milliseconds that secondary eye waits before asserting reset

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
  uint32_t millisec_now;
  pinMode(MOTION_SENSOR_PIN, INPUT_PULLUP);     // input HIGH momentarily when detect person; PIR sensor (D2)
  pinMode(DISPLAY_FORCE_ON_PIN, INPUT_PULLUP);  // input LOW if FORCE ALWAYS-ON
  pinMode(DISPLAY_FORCE_ON_LED_PIN, OUTPUT);    // output HIGH to LED if FORCE ALWAYS-ON
  pinMode(SCNDEYE_DSPLY_ON_PIN, OUTPUT);        // output mirror Display On to other HalloWing if we are primary HalloWing
  pinMode(SCNDEYE_1ST_EYE_PIN, INPUT_PULLUP);   // input HIGH if primary HalloWing; LOW if secondary HalloWing
  digitalWrite(DISPLAY_FORCE_ON_LED_PIN, HIGH); // secondary turn off reset to primary eye; will turn on reset later
                                                //     primary will turn button on for short time until user_loop()
  digitalWrite(SCNDEYE_DSPLY_ON_PIN, HIGH);     // not sure how long the rest of setup will take so start HIGH
                                                //     SCNDEYE_DSPLY_ON_PIN unused on 2nd eye

  millisec_now = millis();
  millisec_for_off = MSEC_ON_4_PIR + millisec_now; // we start with display on; this is TRUE first time through loop()
  // setup vars for second HalloWing doing reset of primary HalloWing
  millisec_for_wait_2_reset = MSEC_WAIT_4_RESET + millisec_now;
  millisec_for_reset_off = MSEC_ON_4_RESET + millisec_for_wait_2_reset;
}  // end user_setup()

//-----------------------------------------------------------------------------
//
// user_loop_primary() - do processing for primary Hallowing M4
//
//    Primary eye has no connect for SCNDEYE_1ST_EYE_PIN so it reads HIGH (due to internal pullup)
//       2nd eye has SCNDEYE_1ST_EYE_PIN connected to ground so it reads LOW
//
//    Primary eye calculates display on/off based on PIR and DISPLAY_FORCE_ON_PIN; sends to SCNDEYE_DSPLY_ON_PIN;
//       displays state of FORCE ALWAYS-ON to button LED using DISPLAY_FORCE_ON_LED_PIN.
//
// This primary code checks our input pins and the screen timeout
//    It sets DISPLAY_FORCE_ON_LED_PIN output pin to make the button LED show our FORCE ALWAYS-ON processing state
//    It determines if the screen timeout should be extended and does so if needed
//    It checks if the screen timeout expired and turns the screen backlight off if needed
//    It sets the SCNDEYE_DSPLY_ON_PIN to mirror the calculated display on/off value to the secondary HalloWing
//        (if present)
//
void user_loop_primary(uint32_t millisec_now, uint8_t force_dsply_always_on, uint8_t pir_value) {

  // set button LED to show state of FORCE ALWAYS-ON
  if (force_dsply_always_on) {
    digitalWrite(DISPLAY_FORCE_ON_LED_PIN, HIGH);
  } else {
    digitalWrite(DISPLAY_FORCE_ON_LED_PIN, LOW);
  }

  // determine if we should extend  millisec_for_off - I know, could include with "if" above
  if (force_dsply_always_on) {
    millisec_for_off = MSEC_ON_4_ALWAYS + millisec_now; // rapid refresh so respond quickly if user turns button off
  } else if (HIGH == pir_value) {
    millisec_for_off = MSEC_ON_4_PIR + millisec_now; // PIR detect causes long timeout
  }

  // if later than millisec_for_off, turn backlight off otherwise turn it on
  // NOTE: it will be more than 49 days before we overflow the millisec counts;
  //   battery would be discharged long before that
  // TODO - Will need to handle wraparound if powering with wall power.
  //
  // if we are the primary HalloWing and connected to PIR sensor
  //    use timer and mirror the display-on value to the other one.
  //    If there is only one Hallowing that is fine, it won't hurt to
  //    do this.
  //
  if (millisec_now > millisec_for_off) {
    arcada.setBacklight(0); // turn screen off
    digitalWrite(SCNDEYE_DSPLY_ON_PIN, LOW);
  } else {
    arcada.setBacklight(255); // turn screen on
    digitalWrite(SCNDEYE_DSPLY_ON_PIN, HIGH);
  }

}  // end user_loop_primary()

//-----------------------------------------------------------------------------
//
// user_loop_secondary() - do processing for secondary Hallowing M4
//
//    2nd eye has SCNDEYE_1ST_EYE_PIN connected to ground so it reads LOW
//       Primary eye has no connect for SCNDEYE_1ST_EYE_PIN so it reads HIGH (due to internal pullup)
//
//    This part of secondary eye code does two things:
//       Reset Primary eye a short time after bootup
//       Read instruction from primary eye to turn our display on or off
//
// Optional 2nd Eye code uses pins SCNDEYE_1ST_EYE_PIN (input), DISPLAY_FORCE_ON_PIN (input),
//       and DISPLAY_FORCE_ON_LED_PIN (output)
//   At a certain time past bootup set DISPLAY_FORCE_ON_LED_PIN to LOW for a short time to reset
//       the Primary Hallowing. Times to do this are set in user_setup().
//   Monitor DISPLAY_FORCE_ON_PIN (from primary SCNDEYE_DSPLY_ON_PIN) and turn our display on or off based on that.
//
void user_loop_secondary(uint32_t millisec_now, uint8_t second_display_off) {

  // take care of possible reset of primary HalloWing
  if (0 != millisec_for_reset_off) {
    // we are secondary HalloWing and have not completed reset of primary yet
    if (millisec_now >= millisec_for_reset_off) {
      // reset is all done
      digitalWrite(DISPLAY_FORCE_ON_LED_PIN, HIGH);
      millisec_for_reset_off = millisec_for_wait_2_reset = 0;
    } else if (millisec_now >= millisec_for_wait_2_reset) {
      // time for reset
      digitalWrite(DISPLAY_FORCE_ON_LED_PIN, LOW);
    } // end if actively resetting primary
  } // end if stll more to do for reset of primary

  // we are 2nd HalloWing; follow screen backlight instructions from primary
  if (second_display_off) {
    arcada.setBacklight(0); // turn screen off
  } else {
    arcada.setBacklight(255); // turn screen on
  } // end turn screen on or off

}  // end user_loop_secondary()

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
// This code reads our input pins and calls the loop code for primary or secondary eye.
//
void user_loop(void) {
  uint32_t millisec_now = millis();
  uint8_t always_sense_or_2nd_display_off = (LOW == digitalRead(DISPLAY_FORCE_ON_PIN));
  uint8_t  pir_value = digitalRead(MOTION_SENSOR_PIN); // input is HIGH ~2 sec. when detect person; PIR sensor (D2)

  if (HIGH == digitalRead(SCNDEYE_1ST_EYE_PIN)) {
    // do this code if primary HalloWing
    user_loop_primary(millisec_now, always_sense_or_2nd_display_off, pir_value);
  } else {
    // do this code if secondary HalloWing
    user_loop_secondary(millisec_now, always_sense_or_2nd_display_off);
  }

}  // end user_loop()

#endif // enable/disable this user file
