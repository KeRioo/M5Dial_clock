#include "driver/temp_sensor.h"
#include "M5Dial.h"
#include <TFT_eSPI.h>
#include "Noto.h"
#include "smallFont.h"
#include "middleFont.h"
#include "bigFont.h"
#include "secFont.h"

class ClockDisplay
{
private:
   M5Canvas img;
   TFT_eSPI tft;
   TFT_eSprite sprite;

   static constexpr int r = 116;
   static constexpr int sx = 120;
   static constexpr int sy = 120;

   float x[360];
   float y[360];
   float px[360];
   float py[360];
   float lx[360];
   float ly[360];

   unsigned short grays[12];
   int start[12];
   int startP[60];

   String days[7] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};

   int angle = 0;
   int lastAngle = 0;
   float circle = 100;
   bool dir = 0;


   void initializeCoordinates()
   {
      constexpr double rad = M_PI / 180.0;
      int b = 0, b2 = 0;

      for (int i = 0; i < 360; i++)
      {
         x[i] = ((r - 20) * cos(rad * i)) + sx;
         y[i] = ((r - 20) * sin(rad * i)) + sy;
         px[i] = (r * cos(rad * i)) + sx;
         py[i] = (r * sin(rad * i)) + sy;
         lx[i] = ((r - 6) * cos(rad * i)) + sx;
         ly[i] = ((r - 6) * sin(rad * i)) + sy;

         if (i % 30 == 0)
            start[b++] = i;
         if (i % 6 == 0)
            startP[b2++] = i;
      }
   }

   void initializeGrayscale()
   {
      int co = 210;
      for (int i = 0; i < 12; i++)
      {
         grays[i] = tft.color565(co, co, co);
         co -= 20;
      }
   }

   void drawClockFace()
   {
      const m5::rtc_datetime_t &dt = M5Dial.Rtc.getDateTime();
      angle = dt.time.seconds * 6;

      if (angle >= 360)
         angle = 0;

      sprite.fillSprite(TFT_BLACK);
      sprite.setTextColor(TFT_WHITE, grays[8]);

      sprite.loadFont(secFont);
      sprite.setTextColor(grays[1], TFT_BLACK);
      sprite.drawString(dt.time.seconds < 10 ? "0" + String(dt.time.seconds) : String(dt.time.seconds), sx, sy - 42);
      sprite.unloadFont();

      sprite.loadFont(smallFont);
      sprite.fillRect(64, 82, 16, 28, grays[8]);
      sprite.fillRect(84, 82, 16, 28, grays[8]);
      sprite.fillRect(144, 82, 16, 28, grays[8]);
      sprite.fillRect(164, 82, 16, 28, grays[8]);
      sprite.setTextColor(0x35D7, TFT_BLACK);
      sprite.drawString("MON", 80, 72);
      sprite.drawString("DAY", 160, 72);
      sprite.unloadFont();

      sprite.loadFont(middleFont);
      sprite.setTextColor(grays[2], grays[8]);
      sprite.drawNumber(dt.date.month, 71, 99, 2); // Changed from drawString to drawNumber
      sprite.drawNumber(dt.date.date, 150, 99, 2); // Changed from drawString to drawNumber
      sprite.unloadFont();

      sprite.loadFont(bigFont);
      sprite.setTextColor(grays[0], TFT_BLACK);

      String m = dt.time.minutes < 10 ? "0" + String(dt.time.minutes) : String(dt.time.minutes);
      String h = dt.time.hours < 10 ? "0" + String(dt.time.hours) : String(dt.time.hours);
      sprite.drawString(h + ":" + m, sx, sy + 32);
      sprite.unloadFont();

      sprite.loadFont(Noto);
      sprite.setTextColor(0xA380, TFT_BLACK);
      sprite.drawString("VOLOS", 120, 190);
      sprite.drawString("***", 120, 114);
      sprite.setTextColor(grays[3], TFT_BLACK);

      for (int i = 0; i < 60; i++)
      {
         int idx = (startP[i] + angle) % 360;
         sprite.fillSmoothCircle(px[idx], py[idx], 1, grays[4], TFT_BLACK);
      }

      for (int i = 0; i < 12; i++)
      {
         int idx = (start[i] + angle) % 360;
         sprite.drawNumber((i * 5), x[idx], y[idx]);
         sprite.drawWedgeLine(px[idx], py[idx], lx[idx], ly[idx], 2, 2, grays[3], TFT_BLACK);
      }

      sprite.drawWedgeLine(sx - 1, sy - 82, sx - 1, sy - 70, 1, 5, 0xA380, TFT_BLACK);
      M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());
   }

public:
   ClockDisplay() : img(&M5Dial.Display), tft(), sprite(&tft) {}

   void begin()
   {
      m5::M5Unified::config_t cfg = M5.config();
      M5Dial.begin(cfg, true, true);
      M5Dial.Rtc.setDateTime({{2023, 10, 25}, {15, 56, 56}});
      sprite.createSprite(240, 240);
      sprite.setSwapBytes(true);
      sprite.setTextDatum(4);

      initializeCoordinates();
      initializeGrayscale();
   }

   void loop()
   {
      drawClockFace();
   }
};

ClockDisplay clockDisplay;

void setup()
{
   clockDisplay.begin();
}

void loop()
{
   clockDisplay.loop();
}
