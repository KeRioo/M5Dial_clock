#ifndef TIME_UPDATE_TASK_H
#define TIME_UPDATE_TASK_H

#include <M5Dial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <WiFi.h> // Include WiFi library for internet connectivity
#include <dialConfig.h> // Include DialConfig for configuration management

class TimeUpdateTask
{
public:
    TimeUpdateTask() : taskHandle(nullptr), running(false) {}
    ~TimeUpdateTask() { stop(); }

    void start()
    {
        if (!running)
        {
            running = true;
            xTaskCreate(taskFunction, "TimeUpdateTask", 4096, this, 1, &taskHandle);
            if (taskHandle == nullptr)
            {
                Serial.println("Failed to create TimeUpdateTask.");
                running = false;
            }
            else
            {
                Serial.println("TimeUpdateTask started.");
            }
        }
    }

    void stop()
    {
        if (running)
        {
            running = false;
            if (taskHandle != nullptr)
            {
                vTaskDelete(taskHandle);
                taskHandle = nullptr;
            }
        }
    }

private:
    static void taskFunction(void *pvParameters)
    {
        TimeUpdateTask *instance = static_cast<TimeUpdateTask *>(pvParameters);
        while (instance->running)
        {
            instance->updateTime();
            vTaskDelay(pdMS_TO_TICKS(3600000)); // Update every hour
        }
        vTaskDelete(nullptr);
    }

    void updateTime()
    {

        bool timeUpdated = false;
        uint8_t retryCount = 0;
        const uint8_t maxRetries = 5; // Maximum number of retries for NTP synchronization
        DialConfig &config = DialConfig::getConfig(); // Get the configuration instance

        while (!timeUpdated && retryCount < maxRetries)        
        {
            if (WiFi.isConnected())
            {
                // Synchronize time with an NTP server
                configTime(0, 0, "pool.ntp.org");
                
                if (config.getUseDaylight())
                {
                    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Central European Time (CET) with DST rules
                }
                else
                {
                    setenv("TZ", "CET-1", 1); // Central European Time (CET) without DST
                }
                tzset(); // Apply the timezone settings

                struct tm timeInfo;
                if (getLocalTime(&timeInfo))
                {
                    timeUpdated = true;
                    m5::rtc_datetime_t rtcDateTime;
                    rtcDateTime.date.year = timeInfo.tm_year + 1900; // tm_year is years since 1900
                    rtcDateTime.date.month = timeInfo.tm_mon + 1;    // tm_mon is months since January (0-11)
                    rtcDateTime.date.date = timeInfo.tm_mday;
                    rtcDateTime.time.hours = timeInfo.tm_hour;
                    rtcDateTime.time.minutes = timeInfo.tm_min;
                    rtcDateTime.time.seconds = timeInfo.tm_sec;
                    M5Dial.Rtc.setDateTime(rtcDateTime); // Update M5Dial RTC with synchronized time
                    Serial.println("RTC updated with NTP time.");
                }
                else
                {
                    Serial.println("Failed to get time from NTP server.");
                }
            }
            else
            {
                Serial.println("Waiting for WiFi connection...");
                vTaskDelay(pdMS_TO_TICKS(60000)); // Wait for 1 minute before checking again
            }
        }

        if (!timeUpdated)
        {
            Serial.println("Failed to synchronize time after multiple attempts.");
        }
    }

    TaskHandle_t taskHandle;
    bool running;
};

#endif // TIME_UPDATE_TASK_H