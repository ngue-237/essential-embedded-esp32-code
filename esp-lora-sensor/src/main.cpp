#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

#define SOIL_MOISTURE_SENSOR_PIN A0
#define LIGHT_SENSOR_PIN A3
#define DHT_SENSOR_PIN A4
#define WATER_SENSOR_PIN A2
#define FIRE_SENSOR_PIN A1
#define SOIL_TEMPERATURE_SENSOR_PIN A5
#define RELAIS_1 12
#define RELAIS_2 10
/** LoRa module parameter*/
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

/** LoRa Frequency*/
#define BAND    868E6  // 915E6
#define PABOOST true
/** LoRa module parameter*/

DHT dht(DHT_SENSOR_PIN, DHT11);

const int dry = 800; 
const int wet = 470; 
const int empty = 6;
const int fill = 572;

OneWire oneWire(SOIL_TEMPERATURE_SENSOR_PIN);
DallasTemperature sensors(&oneWire);


/** Setup Json object*/
StaticJsonDocument<200> doc;
JsonObject dht11 = doc.createNestedObject("dht11");
JsonObject soilMoisture = doc.createNestedObject("soilMoisture");
JsonObject waterSensor = doc.createNestedObject("waterSensor");
JsonObject fireSensor = doc.createNestedObject("fireSensor");
JsonObject ligthSensor = doc.createNestedObject("ligthSensor");
JsonObject soilTemperature = doc.createNestedObject("soilTemperature");

int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode (RELAIS_1,OUTPUT);
  pinMode (RELAIS_2,OUTPUT);
  dht.begin();
  sensors.begin();
  /** LoRa configuration setup*/
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  

}

void loop() {
  //capacitive soil moisture
  int sensorVal = analogRead(SOIL_MOISTURE_SENSOR_PIN);
  int percentageHumididy = map(sensorVal, wet, dry, 100, 0);
  //Serial.println("Soil moisture: " + String(percentageHumididy) + "%");
  if(percentageHumididy>0){
    digitalWrite(RELAIS_1,1);
    delay(100000);
  }else{
    digitalWrite(RELAIS_1,0);
    delay(100000);
  }
  //ligth sensor
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  //Serial.println("Ligth sensor: "+ String(analogValue));

  //fire sensor 
  int fireSensorValue = analogRead(FIRE_SENSOR_PIN);
  //Serial.println("Fire sensor: "+ String(fireSensorValue));

  // water sensor 
  int waterSensorValue = analogRead(WATER_SENSOR_PIN);
  //int tauxRemplissage = map(sensorVal, fill, empty, 100, 0);
  //Serial.println("Water sensor: "+ String(waterSensorValue));
  
  //dht11 sensor
  double temperature = dht.readTemperature();
  if(temperature>10){
    digitalWrite(RELAIS_2,1);
    delay(100000);
  }else{digitalWrite(RELAIS_2,0);
    delay(100000);
    }
  double humidity = dht.readHumidity();
  Serial.println("Temperature: " + String(temperature)+ "°C");
  // Serial.println("Humidity: " + String(humidity)+ "%");

  //ds18b20 sensor
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  
  //Serial.println("Soil Temperature: " + String(temperatureC)+ "°C");
  
  dht11["temperature"] = dht.readTemperature();
  dht11["humidity"] = dht.readHumidity();
  soilMoisture["value"] = map(sensorVal, wet, dry, 100, 0);
  waterSensor["value"] = analogRead(WATER_SENSOR_PIN);
  fireSensor["value"] = analogRead(FIRE_SENSOR_PIN);
  ligthSensor["value"] = analogRead(LIGHT_SENSOR_PIN);
  soilTemperature["value"] = sensors.getTempCByIndex(0);
  serializeJsonPretty(doc, Serial);
  Serial.println();

  Serial.print("Sending packet: ");
  Serial.println(counter);
  String sensors = doc.as<String>();

  Serial.println(sensors);

  // send packet
  LoRa.beginPacket();
  LoRa.print(sensors);
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(20000);
}