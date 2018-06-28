/*
 Basic ESP32 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP32 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP32 Arduino:
  - https://github.com/espressif/arduino-esp32

*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#include <Adafruit_NeoPixel.h>

#define PIN 14
#define NUM_LEDS 10
#define BRIGHTNESS 50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

#ifndef BUILTIN_LED
#define BUILTIN_LED 4
#endif

// Update these with values suitable for your network.

const char *ssid = "...";
const char *password = "MeinIMDinDieburg";
const char *mqtt_server = "m21.cloudmqtt.com";
const char *fresaClient = "mini/0";
const char *user = "ixysflsb";
const char *pass = "yqVLDBTa3h1c";
const char *willTopic = fresaClient;
const char *willMsg = "disconnect";

constexpr uint8_t RST_PIN = 21; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 23;  // Configurable, see typical pin layout above

const int LOCK_PIN = 15;

byte neopix_gamma[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
    2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5,
    5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10,
    10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
    25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
    37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
    51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
    69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
    90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
    115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
    144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
    177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void lockControl(String status)
{
  if (status == "open")
  {
    Serial.println("unlocked");

    // Open Lock
    digitalWrite(LOCK_PIN, HIGH);
    delay(200);
    digitalWrite(LOCK_PIN, LOW);
    mqttOpen();
    ledControl("open");
  }
}

void mqttOpen()
{
  // send OPEN message to mqtt server
  snprintf(msg, 75, "open");
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(fresaClient, msg, true);
}

void mqttClose()
{
  // send OPEN message to mqtt server
  snprintf(msg, 75, "close");
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(fresaClient, msg, true);
}

void ledControl(String status)
{
  if (status == "open")
  {
    Serial.println("leds green");
    // Some example procedures showing how to display to the pixels:
    //colorWipe(strip.Color(255, 0, 0), 50); // Red

    colorWipe(strip.Color(0, 255, 0), 10); // Green

    //colorWipe(strip.Color(0, 0, 255), 50); // Blue
    //colorWipe(strip.Color(0, 0, 0, 255), 50); // White

    //whiteOverRainbow(20,75,5);

    pulseGreenWhite(7);

    //fullWhite();
    //delay(2000);

    //rainbowFade2White(3,3,1);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  payload[length] = '\0';
  String message = String((char *)payload);
  Serial.println();

  if (message == "open lock")
  {
    lockControl("open");
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), user, pass, willTopic, 2, 1, willMsg))
    {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(fresaClient);
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

void rfid()
{
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++)
    key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
      Serial.write(buffer1[i]);
    }
  }
  Serial.print(" ");

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.write(buffer2[i]);
  }

  //----------------------------------------

  Serial.println(F("\n**End Reading**\n"));
  lockControl("open");

  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void pulseGreenWhite(uint8_t wait)
{
  for (int j = 0; j < 256; j++)
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, strip.Color(0, 255, 0, neopix_gamma[j]));
    }
    delay(wait);
    strip.show();
  }

  for (int j = 255; j >= 0; j--)
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, strip.Color(0, 255, 0, neopix_gamma[j]));
    }
    delay(wait + 15j);
    strip.show();
  }
}

void fullWhite()
{

  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 0, 255));
  }
  strip.show();
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(LOCK_PIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 18990);
  client.setCallback(callback);
  SPI.begin();                                               // Init SPI bus
  mfrc522.PCD_Init();                                        // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:")); //shows in serial that it is ready to read
  Serial.println(F("Ready!"));

  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'                            //shows in serial that it is ready to read
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  rfid();
}