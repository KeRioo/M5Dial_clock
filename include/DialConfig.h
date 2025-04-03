#ifndef DIALCONFIG_H
#define DIALCONFIG_H

#include <Arduino.h>
#include <Preferences.h>

class DialConfig {
public:
    struct ConfigData {
        uint8_t brightness;
        bool useDaylight;
        String ssid;
        String password;
        bool isProvisioned;        
    };

    static DialConfig& getConfig() {
        static DialConfig instance; // Singleton instance
        return instance;
    }

    ~DialConfig() {
        preferences.end(); // Close the preferences
    }

    void begin() {
        preferences.begin("dial_config", false); // Namespace for storage
        loadConfig(); // Load configuration on initialization
    }

    void saveConfig() {
        // Save to persistent storage
        preferences.putUChar("brightness", config.brightness);
        preferences.putBool("useDaylight", config.useDaylight);
        preferences.putString("ssid", config.ssid);
        preferences.putString("password", config.password);
        preferences.putBool("isProvisioned", config.isProvisioned);
        preferences.end(); // Close the preferences
    }

    void loadConfig() {
        // Load from persistent storage into in-memory configuration
        config.brightness = preferences.getUChar("brightness", 64); // Default value: 64
        config.useDaylight = preferences.getBool("useDaylight", false); // Default value: false
        config.ssid = preferences.getString("ssid", "aaaaaaaa"); // Default value: empty string
        config.password = preferences.getString("password", "aaaaaaaaaa"); // Default value: empty string
        config.isProvisioned = preferences.getBool("isProvisioned", false); // Default value: false
    }

    // Getters and setters for each parameter
    uint8_t getBrightness() const { return config.brightness; }
    void setBrightness(uint8_t value) { config.brightness = value; saveConfig(); }

    bool getUseDaylight() const { return config.useDaylight; }
    void setUseDaylight(bool value) { config.useDaylight = value; saveConfig(); }

    String getSSID() const { return config.ssid; }
    void setSSID(const String& value) { config.ssid = value; saveConfig(); }

    String getPassword() const { return config.password; }
    void setPassword(const String& value) { config.password = value; saveConfig(); }

    bool getIsProvisioned() const { return config.isProvisioned; }
    void setIsProvisioned(bool value) { config.isProvisioned = value; saveConfig(); }

private:
    Preferences preferences;
    ConfigData config; // In-memory configuration

    DialConfig() {}

    DialConfig(const DialConfig&) = delete; // Prevent copying
    DialConfig& operator=(const DialConfig&) = delete; // Prevent assignment
};

#endif // DIALCONFIG_H