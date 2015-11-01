#include <Phant.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const char WiFiSSID[] = "";
const char WiFiPSK[] = "";

const char PhantHost[] =  "data.sparkfun.com";
const char PublicKey[] = "";
const char PrivateKey[] = "";

#define DHTTYPE DHT22
#define DHTPIN 0

#define ANALOG_PIN A0

const int R1 = 220000;
const int R2 = 51000;
const float divider = (float) R2 / ((float) R1 + (float) R2);
const int correction = 40;

Phant phant(PhantHost, PublicKey, PrivateKey);
DHT_Unified dht(DHTPIN, DHTTYPE);

bool debug = false;

void setup() {
//  debugInit();
  connectWiFi();
  dht.begin();
}

void loop() {
  do {
    if (!readTemperature()) {
      break;
    }
    if (!readHumidity()) {
      break;
    }
    if (!readBatteryVoltage()) {
      break;
    }
    while (!sendToPhant()) {
      delay(100);
    }
  } while (false);
  
  sleepInSeconds(30);
}

void sleepInSeconds(int seconds) {
  ESP.deepSleep(seconds * 1000000);
}

void debugInit() {
   Serial.begin(9600);
   debug = true;
}

bool readTemperature() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
  
  if (isnan(temperature)) {
    if (debug) Serial.println("Sensor read failure (temperature).");
    return false;
  }
  
  phant.add("temperature", temperature);
  if (debug) { Serial.print("Sensor read success (temperature), value: "); Serial.print(temperature); Serial.println(" *C"); }
  return true;
}

bool readHumidity() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  
  if (isnan(humidity)) {
    if (debug) Serial.println("Sensor read failure (humidity).");
    return false;
  }
  
  phant.add("humidity", humidity);
  if (debug) { Serial.print("Sensor read success (humidity), value: "); Serial.print(humidity); Serial.println("%"); }
  return true;
}

bool readBatteryVoltage() {
  float voltage = analogRead(ANALOG_PIN);
  // Correction;
  voltage = voltage - correction;
  voltage = voltage/1000;
  
  // Voltage divider
  voltage = voltage/divider;
  
  phant.add("battery", voltage);
  if (debug) { Serial.print("Analog input: "); Serial.print(voltage); Serial.println(" V"); }
  return true;
}

bool sendToPhant() {
  // Now connect to data.sparkfun.com, and post our data:
  WiFiClient client;
  const int httpPort = 80;
  
  if (!client.connect(PhantHost, httpPort)) 
  {
    // If we fail to connect, return 0.
    return false;
  }
  // If we successfully connected, print our Phant post:
  client.print(phant.post());
  if (debug) Serial.print(phant.post());

  // Read all the lines of the reply from server and print them to Serial
  while(client.available() && debug){
    String line = client.readStringUntil('\r');
    Serial.print(line); // Trying to avoid using serial
  }
  return true;
}

void connectWiFi() {
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);
  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED) {
    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
}
