// ESP32-S3 debug monitor for PC JSON -> I2C LCD
// I2C SDA=8, SCL=9 (ESP32-S3). LiquidCrystal_I2C addr 0x27 by default.

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "IDK 🤷‍♂️";  // Replace with your actual WiFi name
const char* WIFI_PASS = "password";  // Replace with your actual WiFi password
const char* MQTT_SERVER = "192.168.0.249"; // PC LAN IP (not localhost)
const uint16_t MQTT_PORT = 1883;

const int I2C_SDA = 8;
const int I2C_SCL = 9;
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);

float cpu_pct = 0.0;
float mem_pct = 0.0;
unsigned long lastMsg = 0;
int uptime_sec = 0;

void showLCDLine(const char* a, const char* b) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(a);
  lcd.setCursor(0,1);
  lcd.print(b);
}

void showLCDTwo(String l0, String l1) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(l0);
  lcd.setCursor(0,1);
  lcd.print(l1);
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  // parse JSON payload (pc/stats)
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, payload, len);
  if (err) {
    Serial.print("JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }
  cpu_pct = doc["cpu_percent"] | 0.0;
  mem_pct = doc["mem_percent"] | 0.0;
  uptime_sec = doc["uptime"] | 0;
  lastMsg = millis();
  Serial.printf("Parsed CPU=%.1f MEM=%.1f\n", cpu_pct, mem_pct);
}

void mqttConnect() {
  Serial.print("Connecting to MQTT ");
  Serial.print(MQTT_SERVER);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  showLCDTwo("MQTT: connecting", MQTT_SERVER);

  unsigned long start = millis();
  while (!client.connected()) {
    if (client.connect("ESP32S3_PC_DEBUG")) {
      Serial.println("MQTT connected.");
      client.subscribe("pc/stats");
      showLCDTwo("MQTT: connected", "");
      delay(400);
      return;
    } else {
      int rc = client.state();
      Serial.print("MQTT connect failed, rc=");
      Serial.println(rc);
      // show short error numbers to LCD (rc maps from PubSubClient)
      char buf[17];
      snprintf(buf, sizeof(buf), "MQTT err:%d", rc);
      showLCDLine("MQTT failed", buf);
      delay(1500);
      // retry for up to 20s then break to let WiFi re-check
      if (millis() - start > 20000) {
        Serial.println("MQTT connect timeout, will retry after Wi-Fi check.");
        return;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  showLCDLine("PC JSON Monitor","Booting...");
  delay(800);

  // Start WiFi
  Serial.printf("Connecting to WiFi SSID '%s'\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // set MQTT server
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
}

void loop() {
  static unsigned long lastWiFiPrint = 0;
  static unsigned long lastMQTTAttempt = 0;

  // WiFi status reporting
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long t = millis();
    if (t - lastWiFiPrint > 1000) {
      lastWiFiPrint = t;
      wl_status_t s = WiFi.status();
      Serial.print("WiFi status: ");
      Serial.println(s);
      // Display short messages on LCD
      if (s == WL_NO_SSID_AVAIL) showLCDLine("WiFi: SSID not", "available");
      else if (s == WL_CONNECT_FAILED) showLCDLine("WiFi: auth", "failed");
      else if (s == WL_IDLE_STATUS) showLCDLine("WiFi idle", "");
      else showLCDLine("WiFi: connecting","(see serial)");
    }
    // allow WiFi to try connecting, then loop again
    delay(200);
    return;
  }

  // WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    static bool once = false;
    if (!once) {
      once = true;
      IPAddress ip = WiFi.localIP();
      Serial.print("WiFi connected, IP=");
      Serial.println(ip.toString());
      char line0[17], line1[17];
      snprintf(line0, sizeof(line0), "IP:%s", ip.toString().c_str());
      snprintf(line1, sizeof(line1), "GW:%s", WiFi.gatewayIP().toString().c_str());
      showLCDTwo(line0, line1);
      delay(900);
    }
  }

  // MQTT connect if needed (rate-limited)
  if (!client.connected() && millis() - lastMQTTAttempt > 3000) {
    lastMQTTAttempt = millis();
    mqttConnect();
  }

  // keep MQTT alive
  if (client.connected()) {
    client.loop();
    // Update display with latest stats
    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay > 1000) {
      lastDisplay = millis();
      char b0[17], b1[17];
      if (millis() > 10000) {
        // After 10 seconds, show CPU and uptime
        snprintf(b0, sizeof(b0), "CPU: %5.1f %%", cpu_pct);
        int hours = uptime_sec / 3600;
        int minutes = (uptime_sec % 3600) / 60;
        int seconds = uptime_sec % 60;
        snprintf(b1, sizeof(b1), "Up: %02d:%02d:%02d", hours, minutes, seconds);
      } else {
        // First 10 seconds, show CPU and RAM
        snprintf(b0, sizeof(b0), "CPU: %5.1f %%", cpu_pct);
        snprintf(b1, sizeof(b1), "RAM: %5.1f %%", mem_pct);
      }
      lcd.clear();
      lcd.setCursor(0,0); lcd.print(b0);
      lcd.setCursor(0,1); lcd.print(b1);
      Serial.printf("LCD update: %s | %s\n", b0, b1);
    }
  } else {
    // show last known attempt status
    showLCDLine("MQTT notconnected","Retrying...");
  }

  delay(10);
}
