#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

#define PIN 14
#define NUM_LEDS 20
#define BRIGHTNESS 50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

// Update these with values suitable for your network.

const char *ssid = "groot";
const char *password = "test123456";
const char *mqtt_server = "m21.cloudmqtt.com";
const char *fresaClient = "mini/0";
const char *user = "ixysflsb";
const char *pass = "yqVLDBTa3h1c";
const char *willTopic = fresaClient;
const char *willMsg = "disconnect";

String currentState;

constexpr uint8_t RST_PIN = 21; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 23;  // Configurable, see typical pin layout above

const int LOCK_PIN = 15;
const int LOCK_PIN_READ = 32;

int statuss = 0;

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
int lock_state;
bool sent;

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

void lockControl(String state)
{
  if (state == "open")
  {
    Serial.println("unlocked");

    // Open Lock
    digitalWrite(LOCK_PIN, HIGH);
    delay(1000);
    digitalWrite(LOCK_PIN, LOW);
    mqttMsg("open");
    ledControl("open");
  }

  if (state == "open lock")
  {
    Serial.println("unlocked");

    // Open Lock
    digitalWrite(LOCK_PIN, HIGH);
    delay(1000);
    digitalWrite(LOCK_PIN, LOW);
    ledControl("delivery");
  }
}

void mqttMsg(char *data)
{
  // send OPEN message to mqtt server
  snprintf(msg, 75, data);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(fresaClient, msg, true);
}

void ledControl(String status)
{
  if (status == "open")
  {
    Serial.println("leds green");
    startShow(1);
    delay(4000);
    startShow(0);
  }
  else if (status == "delivery")
  {
    startShow(3);
    delay(4000);
    startShow(0);
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
  currentState = message;

  if (currentState == "open lock")
    lockControl(currentState);
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
    if (client.connect(clientId.c_str(), user, pass))
    {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(fresaClient);

      // send OPEN message to mqtt server
      mqttMsg("open");
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
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  if (content.substring(1) == "A4 E3 E6 1E") //change UID of the card that you want to give access
  {
    Serial.println("Access Granted");
    Serial.println("Hallo Sabine Reitmaier");
    delay(1000);
    Serial.println("Nimmm dein Fresa Paket mit!");
    Serial.println();
    statuss = 1;
    lockControl("open");
    //light the LEDS Fresa Style
    //colorWipe(strip.Color(0, 255, 0), 20); // Green

    //pulseGreenWhite(7);
  }

  else
  {
    Serial.println(" Access Denied ");
    delay(3000);
  }
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
    delay(wait + 15);
    strip.show();
  }
}

//TestLib for Turning LED off again afer lock closes
void startShow(int i)
{
  switch (i)
  {
  case 0:
    colorWipe(strip.Color(0, 0, 0), 50); // Black/off
    break;
  case 1:
    colorWipe(strip.Color(0, 255, 0), 20); // Green
    break;
  case 2:
    colorWipe(strip.Color(0, 0, 0, 255), 20); //White
    break;
  case 3:
    colorWipe(strip.Color(0, 0, 255), 20); // Blue
    break;
  }
  strip.show();
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(LOCK_PIN_READ, INPUT_PULLUP);
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
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (currentState == "loaded" || currentState == "open")
  {
    rfid();
  }

  lock_state = digitalRead(LOCK_PIN_READ); // reads the state of the Lock

  if (lock_state == HIGH && sent)
  {

    sent = false;
    // Serial.println("HIGH");
  }

  if (lock_state == LOW && !sent && (currentState == "reserved" || currentState == "open lock"))
  {
    // Serial.println("LOW");
    // send OPEN message to mqtt server
    mqttMsg("loaded");
    sent = true;
  }
  else if (lock_state == LOW && !sent && currentState == "open")
  {
    mqttMsg("empty loaded");
    sent = true;
  }
}