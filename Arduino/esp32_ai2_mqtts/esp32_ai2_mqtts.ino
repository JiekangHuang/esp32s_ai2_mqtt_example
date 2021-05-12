#include "PubSubClient.h"
#include "WiFi.h"
#include <SSLClient.h>

#include "AllTrustAnchors.h"

// WiFi config
const char *ssid     = "iCShop_SP";
const char *password = "spring66chen";

#define ANALOG_PIN 36
#define LED_PIN 23

// 上傳Sensor data頻率
#define UPLOAD_INTERVAL 5000

#define MQTT_BROKER "io.adafruit.com"
#define MQTT_PORT 8883
#define MQTT_USERNAME "<YOUR MQTT Username>"
#define MQTT_PASSWORD "<YOUR MQTT Password>"
#define MQTT_PUB_TOPIC MQTT_USERNAME "/feeds/sensor"
#define MQTT_SUB_TOPIC MQTT_USERNAME "/feeds/io"

WiFiClient   tcpClient;
SSLClient    sslClient(tcpClient, TAs, (size_t)TAs_NUM, A0);
PubSubClient mqttsClient(sslClient);

void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttConnect(void);

void setup()
{
    Serial.begin(115200);
    randomSeed(micros());
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println("Started");
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    static unsigned long timer = 0;
    if (!mqttsClient.connected()) {
        mqttConnect();
    }

    if (millis() > timer) {
        timer     = millis() + UPLOAD_INTERVAL;
        int value = analogRead(ANALOG_PIN);
        mqttsClient.publish(MQTT_PUB_TOPIC, String(value).c_str());
        Serial.print("publish: ");
        Serial.println(value);
    }
    mqttsClient.loop();
}

void mqttConnect(void)
{
    mqttsClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttsClient.setCallback(mqttCallback);
    Serial.print(F("Connect to MQTT Broker..."));
    if (mqttsClient.connect(String(random(65535)).c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
        Serial.println(F("Success"));
        mqttsClient.subscribe(MQTT_SUB_TOPIC);
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));
    for (int i = 0; i < (int)length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if ((char)payload[0] == '1') {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}