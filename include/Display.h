#ifndef CLOCK_SETTINGS_FACE_H
#define CLOCK_SETTINGS_FACE_H

#include "M5Dial.h"
#include <TFT_eSPI.h>
#include "fonts/Noto.h"
#include "fonts/smallFont.h"
#include "fonts/middleFont.h"
#include "fonts/bigFont.h"
#include "fonts/secFont.h"

class Display
{
private:
    M5Canvas &img;
    TFT_eSPI &tft;
    TFT_eSprite &sprite;

    // Clock-related variables
    static constexpr int r = 116;
    static constexpr int sx = 120;
    static constexpr int sy = 120;
    float x[360], y[360], px[360], py[360], lx[360], ly[360];
    unsigned short grays[12];
    int start[12], startP[60];
    int angle = 0;

    // Settings-related variables
    int selectedOption = 0;
    static constexpr int totalOptions = 3;
    const char *options[totalOptions] = {"Option 1", "Option 2", "Option 3"};

    // Mode management
    enum Mode { CLOCK, SETTINGS } currentMode = CLOCK;

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
        sprite.fillRect(64, 82, 33, 28, grays[8]);
        sprite.fillRect(144, 82, 33, 28, grays[8]);

        sprite.setTextColor(0x35D7, TFT_BLACK);
        sprite.drawString("MON", 80, 72);
        sprite.drawString("DAY", 160, 72);
        sprite.unloadFont();

        sprite.loadFont(middleFont);
        sprite.setTextColor(grays[2], grays[8]);
        sprite.drawNumber(dt.date.month, 80, 99, 2);
        sprite.drawNumber(dt.date.date, 160, 99, 2);
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
            int number = (i <= 9) ? (45 - i * 5) : (55 - (i - 10) * 5);
            sprite.drawNumber(number, x[idx], y[idx]);
            sprite.drawWedgeLine(px[idx], py[idx], lx[idx], ly[idx], 2, 2, grays[3], TFT_BLACK);
        }

        sprite.drawWedgeLine(sx - 1, sy - 82, sx - 1, sy - 70, 1, 5, 0xA380, TFT_BLACK);
        M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());
    }

    void drawSettingsMenu()
    {
        sprite.fillSprite(TFT_BLACK);
        sprite.setTextColor(TFT_WHITE, TFT_BLACK);

        for (int i = 0; i < totalOptions; i++)
        {
            if (i == selectedOption)
            {
                sprite.setTextColor(TFT_GREEN, TFT_BLACK);
            }
            else
            {
                sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            sprite.drawString(options[i], 120, 60 + i * 40);
        }

        M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());
    }

public:
    Display(M5Canvas &img, TFT_eSPI &tft, TFT_eSprite &sprite)
        : img(img), tft(tft), sprite(sprite) {}

    void begin()
    {
        sprite.createSprite(240, 240);
        sprite.setSwapBytes(true);
        sprite.setTextDatum(4);

        initializeCoordinates();
        initializeGrayscale();
    }

    void loop()
    {
        if (M5Dial.BtnA.wasPressed() && currentMode == CLOCK)
        {
            currentMode = SETTINGS;
        }

        if (currentMode == CLOCK)
        {
            static int lastSecond = -1;
            const m5::rtc_datetime_t &dt = M5Dial.Rtc.getDateTime();
            if (dt.time.seconds != lastSecond)
            {
                lastSecond = dt.time.seconds;
                drawClockFace();
            }
        }
        else if (currentMode == SETTINGS)
        {
            static int lastEncoderValue = M5Dial.Encoder.read() / 4;
            int encoderValue = M5Dial.Encoder.read() / 4;

            if (encoderValue != lastEncoderValue)
            {
                selectedOption = (selectedOption + (encoderValue > lastEncoderValue ? 1 : -1) + totalOptions) % totalOptions;
                lastEncoderValue = encoderValue;
            }

            if (M5Dial.BtnA.wasPressed())
            {
                // Handle option selection logic here
            }

            drawSettingsMenu();
        }
    }
};

#endif // CLOCK_SETTINGS_FACE_H
