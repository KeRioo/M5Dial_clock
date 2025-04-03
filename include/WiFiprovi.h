#ifndef __WIFIPROVI_H__
#define __WIFIPROVI_H__

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProv.h>
#include <M5Dial.h>
#include <time.h>

class WiFiProvisioning {
public:
    WiFiProvisioning(const char* pop, const char* service_name, const char* service_key, bool reset_provisioned)
        : pop(pop), service_name(service_name), service_key(service_key), reset_provisioned(reset_provisioned) {}

    void begin() {
        WiFi.onEvent(WiFiProvisioning::SysProvEvent);
        Serial.println("Begin Provisioning using BLE");

        // Sample UUID that user can pass during provisioning using BLE
        uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02};

        WiFiProv.beginProvision(
            WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BLE, WIFI_PROV_SECURITY_1,
            pop, service_name, service_key, uuid, reset_provisioned
        );

        log_d("ble qr");
        WiFiProv.printQR(service_name, pop, "ble");
    }

private:
    static void SysProvEvent(arduino_event_t* sys_event) {
        switch (sys_event->event_id) {
            case ARDUINO_EVENT_WIFI_STA_GOT_IP:
                Serial.print("\nConnected IP address : ");
                Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
                synchronizeTime(); // Synchronize time after connecting to the internet
                break;
            case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                Serial.println("\nDisconnected. Connecting to the AP again...");
                break;
            case ARDUINO_EVENT_PROV_START:
                Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
                break;
            case ARDUINO_EVENT_PROV_CRED_RECV: {
                Serial.println("\nReceived Wi-Fi credentials");
                Serial.print("\tSSID : ");
                Serial.println((const char*)sys_event->event_info.prov_cred_recv.ssid);
                Serial.print("\tPassword : ");
                Serial.println((const char*)sys_event->event_info.prov_cred_recv.password);
                break;
            }
            case ARDUINO_EVENT_PROV_CRED_FAIL: {
                Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
                if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
                    Serial.println("\nWi-Fi AP password incorrect");
                } else {
                    Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
                }
                break;
            }
            case ARDUINO_EVENT_PROV_CRED_SUCCESS:
                Serial.println("\nProvisioning Successful");
                break;
            case ARDUINO_EVENT_PROV_END:
                Serial.println("\nProvisioning Ends");
                break;
            default:
                break;
        }
    }

    static void synchronizeTime() {
        // Configure NTP server and timezone
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
        Serial.println("Synchronizing time with NTP server...");

        // Wait for time to be set
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println("Failed to obtain time");
            return;
        }

        // Print synchronized time
        Serial.println(&timeinfo, "Time synchronized: %A, %B %d %Y %H:%M:%S");

        // Save time to RTC
        m5::rtc_datetime_t rtcTime = M5Dial.Rtc.getDateTime();
        rtcTime.date.year = timeinfo.tm_year + 1900;
        rtcTime.date.month = timeinfo.tm_mon + 1;
        rtcTime.date.date = timeinfo.tm_mday;
        rtcTime.time.hours = timeinfo.tm_hour;
        rtcTime.time.minutes = timeinfo.tm_min;
        rtcTime.time.seconds = timeinfo.tm_sec;

        M5Dial.Rtc.setDateTime(rtcTime);
    }

    const char* pop;
    const char* service_name;
    const char* service_key;
    bool reset_provisioned;
};

#endif // __WIFIPROVI_H__