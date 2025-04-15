#ifndef CLOCK_SETTINGS_FACE_H
#define CLOCK_SETTINGS_FACE_H

#include "M5Dial.h"
#include <TFT_eSPI.h>
#include "fonts/Noto.h"
#include "fonts/smallFont.h"
#include "fonts/middleFont.h"
#include "fonts/bigFont.h"
#include "fonts/secFont.h"
#include "Preferences.h"
#include "RTClib.h"

class Display
{
private:
    M5Canvas &img;
    TFT_eSPI &tft;
    TFT_eSprite &sprite;
    RTC_DS1307 &rtc; // RTC object
    Preferences &preferences; // Preferences object for storing settings

    // Clock-related variables
    static constexpr int r = 116;
    static constexpr int sx = 120;
    static constexpr int sy = 120;
    float x[360], y[360], px[360], py[360], lx[360], ly[360];
    unsigned short grays[12];
    int start[12], startP[60];
    int angle = 0;

    // Settings-related variables
    int highlighedOption = 0;
    uint8_t brightness = preferences.getUInt("brightness", 65); // Default brightness value
    static constexpr int totalOptions = 3;
    const char *options[totalOptions] = {"Brightness", "Manual Time Set", "Back"};

    DateTime now = rtc.now();


    // Mode management
    enum Mode
    {
        CLOCK,
        SETTINGS_MENU,
        SET_BRIGHTNESS,
        SET_TIME
    } currentMode = CLOCK;

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
        now = rtc.now();
        angle = now.second() * 6;

        if (angle >= 360)
            angle = 0;

        sprite.fillSprite(TFT_BLACK);
        sprite.setTextColor(TFT_WHITE, grays[8]);

        sprite.loadFont(secFont);
        sprite.setTextColor(grays[1], TFT_BLACK);
        sprite.drawString(now.second() < 10 ? "0" + String(now.second()) : String(now.second()), sx, sy - 42);
        sprite.unloadFont();

        sprite.loadFont(bigFont);
        sprite.setTextColor(grays[0], TFT_BLACK);

        String m = now.minute() < 10 ? "0" + String(now.minute()) : String(now.minute());
        String h = now.hour() < 10 ? "0" + String(now.hour()) : String(now.hour());
        sprite.drawString(h + ":" + m, sx, sy + 32);
        sprite.unloadFont();

        sprite.loadFont(Noto);
        sprite.setTextColor(0xA380, TFT_BLACK);
        sprite.drawString("APLISENS", 120, 190);
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
        static int lastEncoderValue = M5Dial.Encoder.read() / 4;
        int encoderValue = M5Dial.Encoder.read() / 4;

        if (encoderValue != lastEncoderValue)
        {
            highlighedOption = (highlighedOption + (encoderValue > lastEncoderValue ? 1 : -1) + totalOptions) % totalOptions;
            lastEncoderValue = encoderValue;
        }

        if (M5Dial.BtnA.wasPressed())
        {
            switch (highlighedOption)
            {
            case 0: // Brightness
                currentMode = SET_BRIGHTNESS;
                break;
            case 1: // Manual Time Set
                currentMode = SET_TIME;

                break;
            case 2: // Back
                currentMode = CLOCK;
                highlighedOption = 0;
                break;
            default:
                break;
            }
        }
        sprite.fillSprite(TFT_BLACK);
        sprite.loadFont(middleFont);
        sprite.setTextColor(TFT_WHITE, TFT_BLACK);


        for (int i = 0; i < totalOptions; i++)
        {
            if (i == highlighedOption)
            {
                sprite.setTextColor(TFT_ORANGE, TFT_BLACK);
            }
            else
            {
                sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            
            sprite.drawCentreString(options[i], 120, 60 + i * 50, 1);
        }

        M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());
    }

    void drawBrightnessPicker()
    {
        static int lastEncoderValue = M5Dial.Encoder.read() / 4;
        static bool needsRedraw = true; // Ensure the screen redraws on the first call

        int encoderValue = M5Dial.Encoder.read() / 4;

        if (encoderValue != lastEncoderValue)
        {
            uint8_t newBrightness = brightness + (encoderValue > lastEncoderValue ? 5 : -5);
            brightness = max((uint8_t)5, min((uint8_t)250, newBrightness)); // Clamp brightness between 5 and 250
            M5Dial.Display.setBrightness(brightness);
            lastEncoderValue = encoderValue;
            needsRedraw = true;
        }

        if (M5Dial.BtnA.wasPressed())
        {
            M5Dial.Display.setBrightness(brightness);
            preferences.putUInt("brightness", brightness); // Save brightness to preferences
            currentMode = SETTINGS_MENU; // Return to settings menu on button press
            needsRedraw = true;
            return; // No need to redraw as we are exiting
        }

        if (needsRedraw)
        {
            sprite.fillSprite(TFT_BLACK);
            sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString("Brightness", 120, 200, 2); // Text at the bottom

            // Draw the arc for brightness
            int startAngle = 30;  // Start angle in degrees
            int endAngle = 330;   // End angle in degrees (after rotation)
            int radiusOuter = 120; // Outer radius of the arc
            int radiusInner = 105; // Inner radius of the arc
            int cx = 120, cy = 120; // Center of the circle

            // Map brightness to the angle range
            int angle = map(brightness, 0, 255, startAngle, endAngle);

            // Draw the arc background
            sprite.drawSmoothArc(cx, cy, radiusInner, radiusOuter, startAngle, endAngle, TFT_DARKGREY, TFT_BLACK, true);

            // Draw the filled arc for the current brightness
            sprite.drawSmoothArc(cx, cy, radiusInner, radiusOuter, startAngle, angle, TFT_ORANGE, TFT_BLACK, true);

            // Display the brightness value in the center
            sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString(String(map(brightness, 5, 250, 2, 100)) + "%", 120, 120, 9);

            M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());

            needsRedraw = false; // Reset the redraw flag after drawing
        }
    }

    struct TimePickerState
    {
        int selectedField = 0; // 0: Hour, 1: Minute, 2: Second, 3: Back
        bool editMode = false;
        bool lastEditMode = false;
        int lastEncoderValue = M5Dial.Encoder.read() / 4;
        unsigned long lastRedrawTime = 0; // Track last redraw time
    } tpState;

    void drawTimePicker()
    {
        now = rtc.now(); // Get the current time
        int encoderValue = M5Dial.Encoder.read() / 4;
        bool needsRedraw = false;

        static int hour = now.hour();
        static int minute = now.minute();
        static int second = now.second();

        if (!tpState.editMode && encoderValue != tpState.lastEncoderValue)
        {
            int delta = (encoderValue > tpState.lastEncoderValue) ? 1 : -1;
            tpState.lastEncoderValue = encoderValue;

            tpState.selectedField = (tpState.selectedField + delta + 4) % 4; // Cycle through fields
            needsRedraw = true;
        }
        else if (tpState.editMode && encoderValue != tpState.lastEncoderValue)
        {
            int delta = (encoderValue > tpState.lastEncoderValue) ? 1 : -1;
            tpState.lastEncoderValue = encoderValue;

            switch (tpState.selectedField)
            {
            case 0: // Adjust hour
                hour = (hour + delta + 24) % 24; // Wrap around 0-23
                break;
            case 1: // Adjust minute
                minute += delta;
                if (minute >= 60)
                {
                    minute = 0;
                    hour = (hour + 1) % 24; // Increment hour
                }
                else if (minute < 0)
                {
                    minute = 59;
                    hour = (hour - 1 + 24) % 24; // Decrement hour
                }
                break;
            case 2: // Adjust second
                second += delta;
                if (second >= 60)
                {
                    second = 0;
                    minute++;
                    if (minute >= 60)
                    {
                        minute = 0;
                        hour = (hour + 1) % 24; // Increment hour
                    }
                }
                else if (second < 0)
                {
                    second = 59;
                    minute--;
                    if (minute < 0)
                    {
                        minute = 59;
                        hour = (hour - 1 + 24) % 24; // Decrement hour
                    }
                }
                break;
            default:
                break;
            }
            needsRedraw = true;
        }

        if (M5Dial.BtnA.wasPressed())
        {
            if (tpState.selectedField == 3 && !tpState.editMode) // Back field in highlight mode
            {
                currentMode = SETTINGS_MENU; // Return to settings menu
                return; // No need to redraw as we are exiting
            }
            else if (!tpState.editMode) // Enter edit mode
            {
                tpState.editMode = true;
            }
            else // Exit edit mode
            {
                tpState.editMode = false;
                if (tpState.selectedField != 3) // Save time only if not on Back field
                {
                    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
                }
            }
            needsRedraw = true;
        }

        // Allow time flow in edit mode
        if (!tpState.editMode)
        {
            now = rtc.now();
            hour = now.hour();
            minute = now.minute();
            second = now.second();
        }
        else
        {
            unsigned long currentMillis = millis();
            if (currentMillis - tpState.lastRedrawTime >= 1000)
            {
                second++;
                if (second >= 60)
                {
                    second = 0;
                    minute++;
                    if (minute >= 60)
                    {
                        minute = 0;
                        hour = (hour + 1) % 24; // Increment hour
                    }
                }
                tpState.lastRedrawTime = currentMillis;
                needsRedraw = true;
            }
        }

        // Redraw every second or if something has changed
        unsigned long currentMillis = millis();
        if (needsRedraw || tpState.editMode != tpState.lastEditMode || currentMillis - tpState.lastRedrawTime >= 1000)
        {
            sprite.fillSprite(TFT_BLACK);
            sprite.setTextColor(TFT_WHITE, TFT_BLACK);
            sprite.loadFont(middleFont);
            sprite.drawCentreString("Time Picker", 120, 30, 1);

            // Display the time with the selected field highlighted or in edit mode
            String h = hour < 10 ? "0" + String(hour) : String(hour);
            String m = minute < 10 ? "0" + String(minute) : String(minute);
            String s = second < 10 ? "0" + String(second) : String(second);

            sprite.setTextColor(tpState.selectedField == 0 ? (tpState.editMode ? TFT_RED : TFT_ORANGE) : TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString(h, 60, 120, 4);

            sprite.setTextColor(tpState.selectedField == 1 ? (tpState.editMode ? TFT_RED : TFT_ORANGE) : TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString(":", 100, 120, 4);
            sprite.drawCentreString(m, 120, 120, 4);

            sprite.setTextColor(tpState.selectedField == 2 ? (tpState.editMode ? TFT_RED : TFT_ORANGE) : TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString(":", 160, 120, 4);
            sprite.drawCentreString(s, 180, 120, 4);

            sprite.setTextColor(tpState.selectedField == 3 ? TFT_ORANGE : TFT_WHITE, TFT_BLACK);
            sprite.drawCentreString("Back", 120, 180, 2);

            sprite.unloadFont();
            M5Dial.Display.pushImage(0, 0, 240, 240, (uint16_t *)sprite.getPointer());

            // Update last known states
            tpState.lastEditMode = tpState.editMode;
            tpState.lastRedrawTime = currentMillis;
        }
    }

public:
    Display(M5Canvas &img, TFT_eSprite &sprite, TFT_eSPI &tft, RTC_DS1307 &rtc, Preferences &preferences)
        : img(img), tft(tft), sprite(sprite), rtc(rtc), preferences(preferences) {}

    void begin()
    {
        sprite.createSprite(240, 240);
        sprite.setSwapBytes(true);
        sprite.setTextDatum(4);
        M5Dial.Display.setBrightness(64);

        initializeCoordinates();
        initializeGrayscale();
    }

    void loop()
    {
        if (M5Dial.BtnA.wasPressed() && currentMode == CLOCK)
        {
            currentMode = SETTINGS_MENU;
            return;
        }
        switch (currentMode)
        {
        case CLOCK:
        {
            static int lastSecond = -1;
            now = rtc.now();
            if (now.second() != lastSecond)
            {
                lastSecond = now.second();
                drawClockFace();
            }
        }
        break;

        case SETTINGS_MENU:
            drawSettingsMenu();
            break;

        case SET_BRIGHTNESS:
            drawBrightnessPicker();
            break;
        case SET_TIME:
            drawTimePicker();
            break;
        default:
            break;
        }
    }
};

#endif // CLOCK_SETTINGS_FACE_H
