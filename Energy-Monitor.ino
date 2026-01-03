/*
 * Monitoring data read from MQTT Server
 *
 * Thanx to https://esp32io.com/tutorials/esp32-led-matrix for details (instruction and wiring diagram).
 *
 */

// Watchdog relevant
#include "esp_system.h"
#include "rom/ets_sys.h"

// Monitor relevant
#include <config.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <WiFi.h>
// Doku: http://pubsubclient.knolleary.net/api
#include <PubSubClient.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES_NUMBER 4  // 4 blocks

#define CS_PIN_1 17  // Row 1
#define CS_PIN_2 19  // Row 2
#define CS_PIN_3 21  // Row 3
#define CS_PIN_4 22  // Row 4

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;

MD_Parola ledMatrix[] = { MD_Parola(HARDWARE_TYPE, CS_PIN_1, MAX_DEVICES_NUMBER),
                          MD_Parola(HARDWARE_TYPE, CS_PIN_2, MAX_DEVICES_NUMBER),
                          MD_Parola(HARDWARE_TYPE, CS_PIN_3, MAX_DEVICES_NUMBER),
                          MD_Parola(HARDWARE_TYPE, CS_PIN_4, MAX_DEVICES_NUMBER) };

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// MQTT Data
String mqttPreis = "?";
String mqttHausEnergyConsumption = "?";
String mqttTibberEnergyConsumption = "?";
String mqttHausPvDach = "?";

// watchdog
const int wdtTimeout = 30000;  // timeout in µs
hw_timer_t* timer = NULL;

// =========== Watchdog ==============

/**
 * Calles on watchdog alarm.
 */
void ARDUINO_ISR_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

// =========== MQTT ==============

/**
 * Called when new mqtt message was received.
 *
 */
void mqtt_callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic [");
  Serial.print(topic);
  Serial.print("]. Message: [");

  String strValue;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    strValue += (char)message[i];
  }
  Serial.println("]");

  if (strcmp(topic, "/home/tibber/price/current") == 0) {
    mqttPreis = strValue;
    Serial.println("FOUND");
  }
  if (strcmp(topic, "/home/haus/energy/consumption") == 0)
    mqttHausEnergyConsumption = strValue;
}

/**
 * Check mqtt connection and try to reconnect if connection was lost or not etsablished yet.
 *
 */
void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("ESP32-EnergyMonitor", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");

      // Subscribe
      mqtt_client.subscribe("/home/haus/energy/consumption");
      mqtt_client.subscribe("/home/tibber/pulse/power/value/watt");
      mqtt_client.subscribe("/home/tibber/price/current");
      mqtt_client.subscribe("/home/victron/mp/soc");
      mqtt_client.subscribe("/home/victron/mp/power");
      mqtt_client.subscribe("/home/haus/pv/dach");


    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// =========== Setup ==============

void setup() {

  // Initialize Watchdog
  timer = timerBegin(1000000);                     // timer 1Mhz resolution
  timerAttachInterrupt(timer, &resetModule);       // attach callback
  timerAlarm(timer, wdtTimeout * 1000, false, 0);  // set timeout as wdtTimeout (µs)

  delay(200);
  Serial.begin(115200);
  delay(500);

  // LED off,clear and low light
  for (int i = 0; i < MAX_DEVICES_NUMBER; i++) {
    ledMatrix[i].begin();          // initialize the LED Matrix
    ledMatrix[i].setIntensity(1);  // set the brightness of the LED matrix display (from 0 to 15)
    ledMatrix[i].displayClear();   // clear LED matrix display
  }

  // all left
  ledMatrix[0].setTextAlignment(PA_LEFT);
  ledMatrix[0].print("Start");

  // Wifi connect
  //WiFi.mode(WIFI_STA);  //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");
  ledMatrix[0].print("Warte");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  // --- WIFI is connected ----
  ledMatrix[0].print("WLAN ok");
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  String s = WiFi.localIP().toString();
  Serial.println(s);

  // Show IP
  //ledMatrix[0].displayClear();
  ledMatrix[1].displayText(s.c_str(), PA_CENTER, 20, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!ledMatrix[1].displayAnimate())
    ;

  ledMatrix[2].print("MQTT");
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);

  ledMatrix[3].print("Ready");
  delay(1000);

  ledMatrix[0].setTextAlignment(PA_LEFT);
  ledMatrix[1].setTextAlignment(PA_LEFT);
  ledMatrix[2].setTextAlignment(PA_LEFT);
  ledMatrix[3].setTextAlignment(PA_LEFT);
}

// =========== Loop ==============


void loop() {
  timerWrite(timer, 0);  // feed watchdog
  //esp_task_wdt_reset();

  /*  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }*/

  mqtt_reconnect();  //MQTT

  for (int i = 0; i < MAX_DEVICES_NUMBER; i++) ledMatrix[i].displayClear();
  ledMatrix[0].print("Verbrauch");
  ledMatrix[1].print(mqttHausEnergyConsumption);
  ledMatrix[2].print("Preis");
  ledMatrix[3].print(mqttPreis);

  // updateTime();
  delay(1000);
  mqtt_client.loop();
}
