#include "Arduino.h"
#include "M5Dial.h"
#include "Display.h"
#include "WiFiProvi.h"
#include "TimeUpdateTask.h"
#include "DialConfig.h"

M5Canvas img(&M5Dial.Display);
TFT_eSPI tft;
TFT_eSprite sprite(&tft);
Display display(img, tft, sprite);
WiFiProvisioning wifiProvisioning("1234", "Prov_M5Dial", "");
TimeUpdateTask timeUpdateTask;

bool inSettingsMenu = false;

void setup()
{
   Serial.begin(115200);
   while (!Serial);
   m5::M5Unified::config_t cfg = M5.config();
   M5Dial.begin(cfg, true, false);
   M5Dial.update();
   DialConfig::getConfig().begin();
   DialConfig &config = DialConfig::getConfig();
   config.saveConfig();

   if (config.getSSID() != "aaaaaaaa")
   {
      WiFi.begin(config.getSSID().c_str(), config.getPassword().c_str());
      Serial.println("Connecting to WiFi...");
      while (WiFi.status() != WL_CONNECTED)
      {
         delay(1000);
         Serial.println("Connecting to WiFi...");
      }
      Serial.println("Connected to WiFi!");
   }
   else
   {
      Serial.println("No WiFi credentials found. Starting provisioning.");
      wifiProvisioning.begin();
   }
   timeUpdateTask.start();
   display.begin();

}

void loop()
{
   M5Dial.update();
   display.loop();
}
