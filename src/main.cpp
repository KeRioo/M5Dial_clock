#include "M5Dial.h"
#include "Display.h"
#include "WiFiProvi.h"

M5Canvas img(&M5Dial.Display);
TFT_eSPI tft;
TFT_eSprite sprite(&tft);


Display display(img, tft, sprite);
WiFiProvisioning wifiProvisioning("1234", "Prov_M5Dial", "", true);

bool inSettingsMenu = false;

void setup()
{
   Serial.begin(115200);
   m5::M5Unified::config_t cfg = M5.config();
   M5Dial.begin(cfg, true, false);

   wifiProvisioning.begin();
   display.begin();
}

void loop()
{
   M5Dial.update();
   display.loop();
}
