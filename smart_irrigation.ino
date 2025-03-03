#define BLYNK_TEMPLATE_ID "TMPL3GZkFQSfj"
#define BLYNK_TEMPLATE_NAME "Automatic Irrigation System"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define SENSOR_PIN A0  
#define RELAY_PIN D0  // Relay connected to D0 (GPIO16 on NodeMCU)

// WiFi & Blynk credentials
char ssid[] = "Galaxy";
char pass[] = "12345678";
char auth[] = "riXrsM19uP6OGLES0UGKpJdiEvGbNxq1";

// Sensor calibration values
const int dryValue = 1024;
const int wetValue = 460;

// Pump Control Variables
const int moistureThresholdLow = 30;
const int moistureThresholdHigh = 80;
bool pumpState = false;
bool manualMode = false; 

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
    Serial.begin(9600);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH); // Ensure pump is OFF initially

    lcd.init();      
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print("WiFi Connecting...");
    Serial.print("Connecting to WiFi...");

    WiFi.begin(ssid, pass);
    int attempt = 0;

    while (WiFi.status() != WL_CONNECTED && attempt < 15) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Connected");
    } else {
        Serial.println("\nWiFi Error!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi Error!");
        while (true);
    }

    lcd.setCursor(0, 1);
    lcd.print("Blynk Connecting...");
    Serial.println("Connecting to Blynk...");

    Blynk.config(auth);
    delay(2000);

    if (Blynk.connect(3000)) { 
        Serial.println("Blynk Connected!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Blynk Connected");
    } else {
        Serial.println("Blynk Error!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Blynk Error!");
        while (true);
    }

    delay(2000);
    lcd.clear();
}

void loop() {
    Blynk.run(); 
    
    int sensorValue = analogRead(SENSOR_PIN);
    int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100); 

    Serial.print("Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(moisturePercent);
    lcd.print("%  ");

    Blynk.virtualWrite(V0, moisturePercent);

    if (!manualMode) { 
        if (moisturePercent < moistureThresholdLow && !pumpState) {
            digitalWrite(RELAY_PIN, LOW);
            pumpState = true;
            Serial.println("Pump ON (Auto)");
        } 
        else if (moisturePercent > moistureThresholdHigh && pumpState) {
            digitalWrite(RELAY_PIN, HIGH);
            pumpState = false;
            Serial.println("Pump OFF (Auto)");
        }
    }

    lcd.setCursor(0, 1);
    lcd.print("Pump: ");
    lcd.print(pumpState ? "ON  " : "OFF ");

    delay(1000);
}

// Blynk button control (V1) - Manually turn pump ON/OFF
BLYNK_WRITE(V1) {
    int buttonState = param.asInt();

    if (buttonState == 1) { 
        manualMode = true; 
        digitalWrite(RELAY_PIN, LOW);
        pumpState = true;
        Serial.println("Pump ON (Manual)");
    } else { 
        manualMode = false; 
        Serial.println("Returning to Auto Mode");

        int sensorValue = analogRead(SENSOR_PIN);
        int moisturePercent = map(sensorValue, dryValue, wetValue, 0, 100);
        moisturePercent = constrain(moisturePercent, 0, 100);

        if (moisturePercent > moistureThresholdHigh) {
            digitalWrite(RELAY_PIN, HIGH);
            pumpState = false;
            Serial.println("Pump OFF (Auto Recheck)");
        }
    }
}
