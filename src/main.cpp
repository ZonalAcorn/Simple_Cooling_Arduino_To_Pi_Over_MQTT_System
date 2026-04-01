#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHT_PIN 2
#define DHT_TYPE DHT11
#define GREEN_LED 12
#define RED_LED 13
#define MOTOR 10
#define sensor_pin A1

#define HIGH_TEMP_THRESHOLD 75
#define LOW_TEMP_THRESHOLD 70


//initialize DHT sensor
DHT dht(DHT_PIN, DHT11);
int sensor;
const int threshold = 100;
//variable for motor speed
int Motor_Speed;
int Fan;
int Light;


void setup() {
  // put your setup code here, to run once:
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  pinMode(MOTOR, OUTPUT);
  Fan = 0;
  Serial.begin(9600);
  //Serial.println("Temp monitoring system active");
  dht.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  float temperature = dht.readTemperature(true);
  float hum = dht.readHumidity();
  sensor = analogRead(sensor_pin);
  delay(1000);
  if (isnan(temperature)) {
    Serial.println("Failed to read DHT sensor");
    return;
  }
  //Serial.println(temperature);
  //printJsonValue()


  //first threshold, when light is off
  if (sensor<threshold) {
    //Serial.println("Light Off");
    Light = 0;
    if (temperature<68) {
      Motor_Speed = 0;
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, HIGH);
      Fan = 0;
    }
    if (temperature>68 and temperature<69) {
      Motor_Speed = 100;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    if (temperature>69 and temperature<71) {
      Motor_Speed = 200;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    if (temperature>71) {
      Motor_Speed = 255;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    //Serial.println(Motor_Speed);
  }


  //Second threshold (when light is on)
  if (sensor>threshold) {
    //Serial.println("Light On");
    Light = 1;
    if (temperature<72) {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      Fan = 0;
    }
    if (temperature>72 and temperature<73) {
      Motor_Speed = 100;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    if (temperature>73 and temperature<75) {
      Motor_Speed = 200;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    if (temperature>75) {
      Motor_Speed = 255;
      analogWrite(MOTOR, Motor_Speed);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Fan = 1;
    }
    //Serial.println(Motor_Speed);
  }
  
  //Create a JSON document and record all data as key value pairs, then serialize
  JsonDocument doc;
  doc["DeviceId"] = "Bob_Gateway";
  doc["LightStatus"] = Light;
  doc["LightLevel"] = sensor;
  doc["Temperature"] = temperature;
  doc["Humidity"] = hum;
  doc["FanStatus"] = Fan;
  doc["FanSpeed"] = Motor_Speed;
  serializeJson(doc, Serial);
  delay(10000);
}