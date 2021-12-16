#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include <Adafruit_I2CDevice.h>
#include <PubSubClient.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include "secrets.h"

int status = WL_IDLE_STATUS; // the WiFi radio's status

MKRIoTCarrier carrier;
WiFiClient wificlient;
PubSubClient client(wificlient);

// bool disableChecking = false;

uint32_t red = carrier.leds.Color(0, 255, 0);
uint32_t green = carrier.leds.Color(255, 0, 0);
uint32_t yellow = carrier.leds.Color(219, 255, 0);

bool autoMode = true;

void connectToWiFi()
{
  status = WiFi.begin(SSID);
  delay(5000);
  if (status != WL_CONNECTED)
  {
    Serial.println("Couldn't connect to WiFi...");
    carrier.Buzzer.sound(220);
    delay(50);
    carrier.Buzzer.noSound();
    delay(20);
    carrier.Buzzer.sound(220);
    delay(100);
    carrier.Buzzer.noSound();
    // carrier.leds.fill(red, 0, 5);
    // carrier.leds.show();
  }
  else
  {
    Serial.println("Connected to WiFi!");
    // carrier.leds.fill(green, 0, 5);
    // carrier.leds.show();
    carrier.Buzzer.sound(440);
    delay(100);
    carrier.Buzzer.noSound();
  }
}

void reconnectMQTTClient()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(CLIENT_NAME.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("Retrying in 3 seconds - failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

void callback(char *topic, uint8_t *payload, unsigned int length)
{
  char buff[length + 1];
  for (int i = 0; i < length; i++)
  {
    buff[i] = (char)payload[i];
  }
  buff[length] = '\0';

  Serial.print("Message received:");
  Serial.println(buff);

  // the original intention was to have the arduino stop sending data if home mode was toggled... but I could not get it working.
  // string message = buff;

  // if (message == "leaving")
  // {
  //   disableChecking = true;
  //   Serial.print("Checking disabled.");
  // }
  // else if (message == "home")
  // {
  //   disableChecking = false;
  //   Serial.print("Checking disabled.");
  // }
}

void createMQTTClient()
{
  client.setServer(BROKER.c_str(), 1883);
  client.setCallback(callback);
  reconnectMQTTClient();
}

void setup()
{
  CARRIER_CASE = true;
  carrier.begin();
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setTextSize(9);
  carrier.display.setTextColor(ST77XX_BLACK);
  
  connectToWiFi();
  createMQTTClient();
  // client.subscribe(CLIENT_TELEMETRY_TOPIC.c_str());
}

void postMessage(string topic, string message)
{
  string telemetry = message;
  const string CLIENT_TELEMETRY_TOPIC = topic;

  client.publish(CLIENT_TELEMETRY_TOPIC.c_str(), telemetry.c_str());
}


void loop()
{
  int none, light;
  while (!carrier.Light.colorAvailable())
  {
    delay(5);
  }

  carrier.Light.readColor(none, none, none, light);
  carrier.display.setCursor(0, 0);
  carrier.display.setTextSize(9);
  carrier.display.print(light);
  carrier.display.setCursor(0, 100);
  carrier.display.setTextSize(3);
  carrier.display.print("BUT0: Manual Toggle");
  carrier.display.setCursor(0, 165);
  carrier.display.print("BUT1: Auto Mode");
  carrier.Buttons.update();
  if (carrier.Buttons.onTouchDown(TOUCH0))
  {
    autoMode = false;
    postMessage("light", "manual");
    carrier.Buzzer.sound(1500);
    delay(10);
    carrier.Buzzer.noSound();
  }
  if (carrier.Buttons.onTouchDown(TOUCH1))
  {
    autoMode = true;
    postMessage("light", "auto");
  }
  postMessage("light", to_string(light));
  delay(150);

  if (autoMode){
    if (light >= 201){
      carrier.display.fillScreen(ST77XX_RED);
    } else if (light <= 200){
      carrier.display.fillScreen(ST77XX_GREEN);
    }
  }
  else if (!autoMode) {
    carrier.display.fillScreen(ST77XX_YELLOW);
  }
}