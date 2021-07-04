#include <Arduino.h>
/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// inclue library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "test.h"
#include "DHT.h"
#include <Adafruit_Sensor.h>

// Zone use macro
#define DHTPIN D3 // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
// Update these with values suitable for your network.
// wif setting
#define DHTTYPE DHT11 // DHT 11

#define MSG_BUFFER_SIZE (50)

// variable declaration
const char *ssid = "win";
const char *password = "125478963";
// mqtt broker
const char *mqtt_server = "test.mosquitto.org";

unsigned long lastMsg = 0;

int led_1 = D4;
int led_2 = D5;
// #define DHTTYPE DHT11 // DHT 11
// #define DHTPIN D3     // Digital pin connected to the DHT sensor
char msg[MSG_BUFFER_SIZE];
char temp_[MSG_BUFFER_SIZE];
char index_temp[MSG_BUFFER_SIZE];

int value = 0;

// instand object
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// Passing by reference and Passing by value
void call_funtion(String my_topic, String msg);

// setup wifi
void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// funtion callback
void callback(char *topic, byte *payload, unsigned int length)
{
  String msg = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();

  String my_Topic(topic);
  // int bottleCount = 0; // assume no bottles unless we correctly parse a value from the topic
  if (my_Topic.indexOf('/') >= 0)
  {
    // The topic includes a '/', we'll try to read the number of bottles from just after that
    my_Topic.remove(0, my_Topic.indexOf('/') + 1);
    // Now see if there's a number of bottles after the '/'
    Serial.println(my_Topic);
  }

  call_funtion(my_Topic, msg);
}

// reconnect wifi
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic_test", "hello world");

      // ... and resubscribe
      client.subscribe("inTopic_test/+");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // set mode pin
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);

  // set buatrate Serial
  Serial.begin(115200);

  // call funtion setup wifi
  setup_wifi();

  // set server mqtt
  client.setServer(mqtt_server, 1883);

  // set funtion callback
  client.setCallback(callback);
  // DHT 11 setup
  dht.begin();
  // setup_time();
  ///////////////////////////
}

// loop funtion

void loop()
{
  // check connection
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // delay without dalay use time count in deloay
  unsigned long now = millis();

  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;

    // read value from method
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    // check value
    if (isnan(h) || isnan(t))
    {
      Serial.println(F("Failed to read from DHT sensor!"));
      h = 0;
      t = 0;
    }

    float hic = dht.computeHeatIndex(t, h, false);
    // snprintf เป็น function สําหรับ ใส่ String ในตัวแปร ตาม format ที่กําหนด และ การระบุ buffer ข้อความ
    // int snprintf(char *, size_t, const char *, ...)
    snprintf(msg, MSG_BUFFER_SIZE, "%.2f", h);
    snprintf(temp_, MSG_BUFFER_SIZE, "%.2f", t);
    snprintf(index_temp, MSG_BUFFER_SIZE, "%.2f", hic);

    // snprintf(msg, MSG_BUFFER_SIZE, "hello world" );

    // Serial.print("Publish message: ");
    Serial.print(msg);
    Serial.print(temp_);
    Serial.print(hic);
    Serial.println();

    // public data
    client.publish("Data_Sensor/Humidity", msg);
    client.publish("Data_Sensor/Temperature", temp_);
    client.publish("Data_Sensor/Index_temp", index_temp);

  }
}
void call_funtion(String my_topic, String msg)
{
  if (my_topic == "led1")
  {
    if (msg == "on")
    {
      digitalWrite(led_1, HIGH); 
      Serial.println("print on");
    }
    else if (msg == "off")
    {
      digitalWrite(led_1, LOW);
      Serial.println("print LOW");
    }
  }
  if (my_topic == "led2")
  {
    if (msg == "on")
    {
      digitalWrite(led_2, HIGH);
    }
    else if (msg == "off")
    {
      digitalWrite(led_2, LOW);
    }
  }
}
// class temp : public DHT
// {

// public:
//   int DHTPIN = 0;
//   int DHTTYPE = 0;
//   temp(uint8_t pin, uint8_t type, uint8_t count = 6) : DHT(uint8_t pin, uint8_t type, uint8_t count = 6);
//   temp dht(DHTPIN, DHTTYPE);
//   dht.begin();
//   void init_dht_11()
//   {
//   }
//   // Reading temperature or humidity takes about 250 milliseconds!
//   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
//   void read_Humidity()
//   {
//     float h = dht.readHumidity();
//     return h;
//   }
//   void read_Temperature()
//   {
//     float t = dht.readTemperature();
//     return t;
//   }
//   void read_temperature_Fahrenheit()
//   {
//     float f = dht.readTemperature(true);
//     return f;
//   }
// };