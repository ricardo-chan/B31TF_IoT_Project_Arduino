#include <NTPClient.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DHT.h> // dht11 temperature and humidity sensor library
#include <WiFiUdp.h>
#include <Servo.h>
#include <string.h>

// Connection Settings
#define FIREBASE_HOST "smartgarden-bc131.firebaseio.com"
#define FIREBASE_AUTH "G4aRSqNi9i9B0wRLbHdfZW91ZBoWMv9CamAtE3i5"
#define WIFI_SSID "HUAWEI AI Cube_4A1C"
#define WIFI_PASSWORD "88541856"

#define DHTPIN D3     // what digital pin we're connected to
#define DHTTYPE DHT22 // select dht type as DHT 11 or DHT22

const long utcOffsetInSeconds = 0;
long int timestamp;

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson payload;

Servo servo;
DHT dht(DHTPIN, DHTTYPE);
int soilMoisture;

//int fan = D3;
//int growlight = D2;
String fireStatus = ""; // led status received from firebase
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup()
{
  Serial.begin(9600);
  delay(1000);
  servo.attach(5); //D1
  servo.write(0);

  delay(2000);
  pinMode(A0, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());               // print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to firebase
  dht.begin();                                  // Start reading dht sensor
  Firebase.setString(firebaseData, "SET_SERVO", "0");         // send initial string of led status
  timeClient.begin();
  //Firebase.setString("GROWLIGHT_STATUS", "OFF");  // send initial string of led status

  payload.add("value", NULL);
  payload.add("timestamp", NULL);
}

void loop()
{
  // Set timestamp
  timeClient.update();
  timestamp = timeClient.getEpochTime();
  payload.set("timestamp", String(timestamp));

  // Analog read and printout
  int soil = analogRead(A0);
  soilMoisture = map(soil, 1024, 512, 0, 100);
  Serial.println("Soil Moisture ADC: " + soil);
  Serial.println("Soil Moisture value: " + String(soilMoisture) + "%");
  delay(1000);

  // Reading temperature and humidity
  double humidity = dht.readHumidity();
  double temperature = dht.readTemperature(); // Read temperature as Celsius (the default)

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.println("Humidity: " + String(humidity));
  Serial.println("Temperature: " + String(temperature) + "°C");
  delay(4000);

  // Preparation and sending of data
  payload.set("value", humidity);
  Firebase.pushJSON(firebaseData, "/DHT11/Humidity", payload);
  payload.set("value", temperature);
  Firebase.pushJSON(firebaseData, "/DHT11/Temperature", payload);
  payload.set("value", soilMoisture);
  Firebase.pushJSON(firebaseData, "/Soil Moisture/Soil", payload);

  // SERVO logic
  Firebase.getString(firebaseData, "SET_SERVO");
  if (soilMoisture <= 35)
  {
    Serial.println("Valve open");
    servo.write(180);
  }
  else
  {
    Serial.println("Valve closed.");
    servo.write(0);
  }
}
