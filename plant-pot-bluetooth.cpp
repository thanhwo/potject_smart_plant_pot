#include <BluetoothSerial.h>
#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

BluetoothSerial SerialBT;

// Blynk info
#define BLYNK_TEMPLATE_ID "TMPL6iWIxyoZn"
#define BLYNK_TEMPLATE_NAME "TEMP"
#define BLYNK_AUTH_TOKEN "Pt1NbKjjmh6VASo9BfmlcSdqT2C55TDQ"

String ssid = "";
String password = "";

#define VIRTUAL_TEMP V0
#define VIRTUAL_HUMID V1                      
#define VIRTUAL_MOIST V2                      
#define VIRTUAL_LED V3
#define VIRTUAL_PUMP V4

#define LED_PIN 4
#define DHTPIN 5        
#define DHTTYPE DHT11   
#define PUMP_PIN 18       
const int moisturePin = 34;  

DHT dht(DHTPIN, DHTTYPE);

void getWiFiCredentialsBluetooth() {
    Serial.println("📡 Bật Bluetooth: 'ESP32_PlantPot'");
    SerialBT.begin("ESP32_PlantPot");
    Serial.println("📲 Gửi: SSID,PASS (qua Bluetooth)");

    while (ssid == "" || password == "") {
        if (SerialBT.available()) {
            String data = SerialBT.readStringUntil('\n');
            int comma = data.indexOf(',');
            if (comma > 0) {
                ssid = data.substring(0, comma);
                password = data.substring(comma + 1);
                ssid.trim();
                password.trim();
                Serial.println("✅ Đã nhận WiFi qua Bluetooth!");
                Serial.print("SSID: "); Serial.println(ssid);
                Serial.print("PASS: "); Serial.println(password);
            }
        }
    }
}

void checkWiFiStatus() {
    switch (WiFi.status()) {
        case WL_NO_SSID_AVAIL:
            Serial.println("❌ SSID không khả dụng! Kiểm tra lại tên mạng.");
            break;
        case WL_CONNECT_FAILED:
            Serial.println("❌ Kết nối thất bại! Kiểm tra SSID hoặc mật khẩu.");
            break;
        case WL_CONNECTION_LOST:
            Serial.println("⚠️ Mất kết nối, thử lại...");
            break;
        case WL_DISCONNECTED:
            Serial.println("❌ Không thể kết nối WiFi!");
            break;
        case WL_CONNECTED:
            Serial.println("✅ WiFi đã kết nối!");
            break;
        default:
            Serial.println("⚠️ Lỗi WiFi không xác định.");
            break;
    }
}

void setup_wifi() {
    WiFi.disconnect(true);
    delay(1000);
    getWiFiCredentialsBluetooth();

    Serial.print("\n🔄 Đang kết nối WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid.c_str(), password.c_str());

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    checkWiFiStatus();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("📶 Địa chỉ IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ Kết nối WiFi thất bại! Kiểm tra lại thông tin.");
    }
}

BLYNK_WRITE(VIRTUAL_PUMP) {
    int pumpState = param.asInt();
    digitalWrite(PUMP_PIN, pumpState);
    Serial.println(pumpState ? "🟢 Máy bơm ĐÃ BẬT!" : "🔴 Máy bơm ĐÃ TẮT!");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    setup_wifi();
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid.c_str(), password.c_str());

    dht.begin();
    pinMode(LED_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
}

void loop() {
    Blynk.run();

    int soilMoistureValue = analogRead(moisturePin);
    int moisture = map(soilMoistureValue, 4095, 0, 0, 100);

    int temperature = dht.readTemperature();
    int humidity = dht.readHumidity();

    Serial.printf("🌡 Temp: %d°C | 💧 Humidity: %d%% | 🌱 Moisture: %d%%\n", temperature, humidity, moisture);
    Blynk.virtualWrite(VIRTUAL_TEMP, temperature);
    Blynk.virtualWrite(VIRTUAL_HUMID, humidity);
    Blynk.virtualWrite(VIRTUAL_MOIST, moisture);

    if (moisture <= 30) {
        digitalWrite(PUMP_PIN, HIGH);
        Serial.println("🟢 Bật bơm nước!");
        Blynk.virtualWrite(VIRTUAL_PUMP, 1);
    } else if (moisture >= 60) {
        digitalWrite(PUMP_PIN, LOW);
        Serial.println("🔴 Tắt bơm nước!");
        Blynk.virtualWrite(VIRTUAL_PUMP, 0);
    }

    if (Serial.available()) {
        char cmd = Serial.read();
        if (cmd == '1') {
            digitalWrite(PUMP_PIN, HIGH);
            Serial.println("🟢 Máy bơm ĐÃ BẬT!");
            Blynk.virtualWrite(VIRTUAL_PUMP, 1);
        } else if (cmd == '0') {
            digitalWrite(PUMP_PIN, LOW);
            Serial.println("🔴 Máy bơm ĐÃ TẮT!");
            Blynk.virtualWrite(VIRTUAL_PUMP, 0);
        }
    }

    delay(2000);
}