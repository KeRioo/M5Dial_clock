#include "Arduino.h"
#include "M5Dial.h"
#include "Display.h"
#include "RTClib.h"
#include "Wire.h"
#include <Preferences.h>

M5Canvas img(&M5Dial.Display);
TFT_eSPI tft;
TFT_eSprite sprite(&tft);
RTC_DS1307 rtc; // define a object of DS1307 class
// Display display(img, tft, sprite, rtc);
Display *display;
Preferences preferences;

void setup()
{
   m5::M5Unified::config_t cfg = M5.config();
   cfg.internal_rtc = false; // Disable internal RTC

   M5Dial.begin(cfg, true, false);
   M5Dial.update();
   preferences.begin("dial_config", false); // Initialize preferences
   M5Dial.Display.setBrightness(preferences.getUInt("brightness", 65)); // Set brightness from preferences
   Wire.begin(SDA, SCL); // Initialize I2C with custom pins
   rtc.begin(&Wire);     // Pass the Wire instance to the RTC object
   if (!rtc.isrunning())
   {
      rtc.adjust(DateTime(F(__DATE__),F(__TIME__))); // Set the RTC to a known date and time
   }

   display = new Display(img, sprite, tft, rtc, preferences);
   display->begin();
}

void loop()
{
   M5Dial.update();
   display->loop();
}
