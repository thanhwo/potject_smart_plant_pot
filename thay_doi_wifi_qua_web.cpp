#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences prefs;
WebServer server(80);

String ssid, password;

// ======= Mạng WiFi mặc định =======
const char* defaultSSID = "Ten Wifi"; //Tên wifi mặc định
const char* defaultPASS = "matkhau"; //Mật khẩu mặc định

// ======= Bật Access Point khi không kết nối được =======
void startAccessPoint() {
  WiFi.softAP("ESP32_Config");
  IPAddress IP = WiFi.softAPIP();
  Serial.println("❌ Không kết nối được WiFi nào.");
  Serial.print("🔧 Mở AP tại IP: ");
  Serial.println(IP);
}

// ======= Kết nối đến WiFi đã lưu =======
void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.print("🔄 Đang kết nối WiFi đã lưu: ");
  Serial.println(ssid);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    Serial.print(".");
    count++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Đã kết nối WiFi!");
    Serial.print("📡 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Kết nối thất bại.");
    startAccessPoint();
  }
}

// ======= Trang web cấu hình WiFi =======
void handleConfigPage() {
  String html = "<h2> Thiet lap Wifi </h2>";
  html += "<form action=\"/save\" method=\"POST\">";
  html += "SSID: <input name=\"ssid\"><br><br>";
  html += "Password: <input name=\"pass\" type=\"password\"><br><br>";
  html += "<input type=\"submit\" value=\"'Luu va ket noi'\">";
  html += "</form>";
  server.send(200, "text/html", html);
}

// ======= Lưu cấu hình WiFi mới và khởi động lại =======
void handleSaveConfig() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String newSsid = server.arg("ssid");
    String newPass = server.arg("pass");

    prefs.begin("wifi", false);
    prefs.putString("ssid", newSsid);
    prefs.putString("pass", newPass);
    prefs.end();

    server.send(200, "text/html", "<p>✅ Da luu! Dang ket noi lai WiFi...</p>");
    delay(1500);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "❗ Thieu du lieeu!");
  }
}

// ======= Thiết lập ban đầu =======
void setup() {
  Serial.begin(115200);

  // 1. Thử kết nối mạng mặc định trước
  Serial.println("📡 Thử kết nối mạng mặc định...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(defaultSSID, defaultPASS);

  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    Serial.print(".");
    count++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Đã kết nối mạng mặc định!");
    Serial.print("📡 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n⚠️ Kết nối mặc định thất bại. Thử WiFi đã lưu...");

    // 2. Đọc WiFi đã lưu từ bộ nhớ
    prefs.begin("wifi", true);
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("pass", "");
    prefs.end();

    // 3. Nếu có dữ liệu đã lưu thì thử kết nối
    if (ssid != "") {
      connectToWiFi();
    } else {
      Serial.println("⚠️ Không có WiFi đã lưu.");
      startAccessPoint();
    }
  }

  // 4. Cấu hình WebServer
  server.on("/", handleConfigPage);
  server.on("/save", HTTP_POST, handleSaveConfig);
  server.begin();
  Serial.println("🌐 Web server đã chạy!");
}

void loop() {
  server.handleClient();
}
