#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

#define DHTPIN 32
#define DHTTYPE DHT11
#define TEMP_LIMIT 30
#define COOLDOWN_MS 15000

const char* ssid     = "YOUR_WiFi_SSID";
const char* password = "YOUR_WiFi_PASSWORD";
const char* apiKey   = "YOUR_API_KEY";

const char* host = "www.circuitdigest.cloud";
const int httpsPort = 443;

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;

unsigned long lastSentTime = 0;

void sendWhatsApp(float temperature)
{
  if (!client.connect(host, httpsPort)) {
    Serial.println("HTTPS connection failed");
    return;
  }

String tempString = String(temperature) + "°C";

String payload =
  "{"
  "\"phone_number\":\"919876543210\","  //Enter registered Mobile number
  "\"template_id\":\"threshold_violation_alert\","
  "\"variables\":{"
    "\"device_name\":\"Living Room Temp Node\","
    "\"parameter\":\"Temperature\","
    "\"measured_value\":\"" + tempString + "\","
    "\"limit\":\"30°C\","
    "\"location\":\"Living Room\""
  "}"
  "}";

  client.println("POST /api/v1/whatsapp/send HTTP/1.1");
  client.println("Host: www.circuitdigest.cloud");
  client.println("Authorization: " + String(apiKey));
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(payload.length());
  client.println();
  client.println(payload);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  Serial.println(client.readString());
  client.stop();
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  client.setInsecure();   // for testing only
}

void loop()
{
  float temperature = dht.readTemperature();
  unsigned long now = millis();

  Serial.print("Temperature: ");
  Serial.println(temperature);

  if (!isnan(temperature) &&
      temperature >= TEMP_LIMIT &&
      now - lastSentTime > COOLDOWN_MS)
  {
    Serial.println("Threshold reached → Sending WhatsApp");
    sendWhatsApp(temperature);
    lastSentTime = now;
  }

  delay(2000);
}