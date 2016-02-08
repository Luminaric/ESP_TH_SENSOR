/*
 * This sketch writes directly on the ESP chip.
 * The ESP8266WiFi.h library is used here.
 * This sensor can be battery powered or use mains power
 * It uses the ESP12 due to the ease of connecting the 
 * necessary ESP_RST pin to GPIO16.
 * The chip is put into sleep between transmissions.
 * The DHT22 is used to get the Temp and Humidity. Battery 
 * voltage is also sampled and transmitted to the MySQL server.
 * The Temp and Humidity are also sent to IO.ADAFRUIT.
 * 
 * Version:   1.6
 * SensorID:  ESP##
 * Platform:  ESP8266
 * Date:      2016-02-08
 * Requires:
 * Hardware:  Uses the DHT22 Temp and Humid sensor. It also
 * uses the Adafruit IO REST API
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "Adafruit_IO_Client.h"
#include "ethernetSettings.h"
#include <DHT.h>

extern "C" {
#include "user_interface.h"
uint16 readvdd33(void);
}

#define MYSQL_OPEN_STRING     "Get /add2.php?f0="//this is the URL for the PI server
#define CLOSE_STRING          "Host: 10.1.1.25 Connection: close"
#define DHTPIN                2
#define DHTTYPE               DHT22
//change as the following as required for different sensor locations
#define SENSORID              "ESP05"
#define AIO_FEED_H            "downstairs-humidity"
#define AIO_FEED_T            "downstairs-temp"

uint16 x;
float t, h;
float v = readvdd33() / 1000.0;
//Depending on what is getting measured, change the line below to set sleep period
const unsigned long sleepTimeS = 600;//in seconds
const unsigned long multiplier = 1000000;//cycles for one second

DHT dht(DHTPIN, DHTTYPE, 16);

//
WiFiClient client;

Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);

Adafruit_IO_Feed humidity = aio.getFeed(AIO_FEED_H);
Adafruit_IO_Feed temperature = aio.getFeed(AIO_FEED_T);

void sendIOAdafruit() {
  char sH[7], sT[7];
  dtostrf(h,5,1,sH);
  dtostrf(t,5,1,sT);
  //send Humidity
  if(humidity.send(sH)) {
    Serial.print(F("Wrote humidity to feed: ")); Serial.println(sH);
  } else  {
    Serial.println(F("Error writing value to Humidity feed!"));
  }
  //send Temperature
  if(temperature.send(sT)) {
    Serial.print(F("Wrote termperature to Temperature feed: ")); Serial.println(sT);
  } else  {
    Serial.println(F("Error writing value to feed!"));
  }
}
  void getData(void) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t) ) {
      Serial.println("Failed to read from DHT sensor!");
      ESP.reset();
    }
    Serial.print("Temp ");Serial.print(t);Serial.print("|");Serial.print("Humidity ");Serial.println(h);
  }
  void sendMySQL(void)  {
    String getString = MYSQL_OPEN_STRING;
    getString += SENSORID;
    getString += "&f1=";
    getString += t;
    getString += "&f2=";
    getString += h;
    getString += "&f3=";
    getString += 0x00;
    getString += "&f4=";
    getString += v;
    Serial.println(getString);
    client.println(getString);
    client.println(CLOSE_STRING);
    client.println();
    client.println();
    client.stop();
  }

void setup() {
  //Setup Serial and report status
  Serial.begin(9600);
  //Init DHT Sensor
  dht.begin();
  //dhtTest();
  Serial.print("DHT using PIN: ");
  Serial.println(DHTPIN);
  //Connect Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    x += 1;
    if (x > 40)   {//this count give the ESP 20 seconds to connect, it usually taks much less.
      Serial.println("Failed to connect to WAP");
      ESP.reset();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Use WiFiClient class to create TCP connections
  if (!client.connect(serverLH, 80)) {
    Serial.println("connection failed");
    ESP.reset();
  } else  {
    getData();
    sendMySQL();
    sendIOAdafruit(); 
  }
  
  delay(1000);
  system_deep_sleep_set_option(0);
  system_deep_sleep((sleepTimeS * multiplier) - micros());
//ESP.deepSleep(60000000, WAKE_RF_DEFAULT);

}

void loop() {
  delay(6000);

}


