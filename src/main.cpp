#include <Arduino.h>
#include "M5_ENV.h"
#include <PubSubClient.h>
#include <Wire.h>
#include "WiFi.h"

// WiFi credentials.
const char* WIFI_SSID = "zzr232";
const char* WIFI_PASS = "kkrT9aTa1!!";
const char* mqtt_server = "10.0.1.101"; //mqtt server

WiFiClient espClient;
PubSubClient client(espClient); //lib required for mqtt
QMP6988 qmp6988;
SHT3X sht30;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing WiFi and connecting...");
  delay(2000);

  qmp6988.init();

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting WIFI...");
  boolean firstTry = true;
  while (WiFi.status() != WL_CONNECTED) {
    if ( !firstTry ) {
      Serial.println("Retrying....");
    } else {
      firstTry = false;
    }
    // Check to see if connecting failed.
    // This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    delay(5000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);//connecting to mqtt server
}

float *getTemperature(float *resultData) {

  float temp = 0.0f;
  while (sht30.get() != 0);
  resultData[0] = sht30.cTemp;
  resultData[1] = sht30.humidity;
  return resultData;
}

#define INTERVAL 60 * 1000

ulong timestamp = -10000000L;

void loop() {
  float tempData[2] = { 0.0f, 0.0f };

  ulong currentMillis = millis();
  if ( currentMillis - timestamp >= INTERVAL ) {

    char buffer[128] = "";
    if (!client.connected()) {
      Serial.println("Connecting to MQTT...");
      while (!client.connect("m5stack", "mqtt-ha", "IC7610B4by!" )) {
        Serial.println("Connection failed....retrying in 5 seconds...");
        sleep(5);
      };
    }
    getTemperature(tempData);
    sprintf(buffer, "{\"temperature\": %.2f}", tempData[0]);
    Serial.print("Publishing payload : ");
    Serial.println(buffer);
    
    client.publish("office/temperature", buffer);
    sprintf(buffer, "{\"humidity\": %.2f}", tempData[1]);
    client.publish("office/humidity", buffer);
    Serial.println(buffer);
    
    timestamp = currentMillis;
  }
  client.loop();
}