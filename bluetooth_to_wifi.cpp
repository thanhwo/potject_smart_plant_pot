#include <BluetoothSerial.h>
#include <WiFi.h>

BluetoothSerial SerialBT;
String ssid = "";
String password = "";

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32"); // Tên thiết bị Bluetooth
  Serial.println("🔵 Bluetooth Started! Gửi: SSID,PASS");

  // Chờ nhận SSID và PASSWORD từ Bluetooth
  while (ssid == "" || password == "") {
    if (SerialBT.available()) {
      String data = SerialBT.readStringUntil('\n');
      int commaIndex = data.indexOf(',');
      if (commaIndex > 0) {
        ssid = data.substring(0, commaIndex);
        password = data.substring(commaIndex + 1);
        ssid.trim();
        password.trim();

        Serial.println("📡 Đã nhận Wi-Fi:");
        Serial.println("SSID: " + ssid);
        Serial.println("PASS: " + password);
      }
    }
  }

  // Kết nối Wi-Fi
  Serial.print("🌐 Đang kết nối Wi-Fi");
  WiFi.begin(ssid.c_str(), password.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Kết nối thành công!");
    Serial.println("IP: " + WiFi.localIP().toString());
    SerialBT.println("✅ Đã kết nối Wi-Fi. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ Kết nối thất bại!");
    SerialBT.println("❌ Không kết nối được Wi-Fi.");
  }
}

void loop() {
  // Không cần làm gì thêm ở đây
}
