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

const char *ssid = "...";
const char *password = "MeinIMDinDieburg";
const char *mqtt_server = "m21.cloudmqtt.com";
const char *fresaClient = "mini/0";
const char *user = "ixysflsb";
const char *pass = "yqVLDBTa3h1c";
const char *willTopic = fresaClient;
const char *willMsg = "disconnect";

String currentState;
String previousState = "disconnect";
bool disconnect = false;

constexpr uint8_t RST_PIN = 21; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 23;  // Configurable, see typical pin layout above

const int LOCK_PIN = 15;
const int LOCK_PIN_READ = 32;

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
bool startup;

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
    ledControl("open");
    digitalWrite(LOCK_PIN, HIGH);
    delay(1000);
    digitalWrite(LOCK_PIN, LOW);
  }

  if (state == "open lock")
  {
    Serial.println("unlocked");

    // Open Lock
    ledControl("delivery");
    digitalWrite(LOCK_PIN, HIGH);
    delay(1000);
    digitalWrite(LOCK_PIN, LOW);
  }
}

void mqttMsg(char *data)
{
  // send OPEN message to mqtt server
  snprintf(msg, 75, data);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(fresaClient, msg, true);
  sent = true;
}

void ledControl(String status)
{
  if (status == "open")
  {
    Serial.println("leds green");
    startShow(1);
    // delay(4000);
  }
  else if (status == "delivery")
  {
    startShow(3);
    // delay(4000);
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

  if (previousState == "disconnect" && !disconnect)
  {
    currentState = "open";
    previousState = "loaded";
    disconnect = true;
  }
  else if (message == "disconnect" && disconnect)
  {
    return;
  }
  else
  {
    if (currentState == "open lock")
    {
      // Skip change of previousState due to bug
    }
    else
    {
      previousState = currentState;
    }
    currentState = message;
  }
  Serial.println(currentState);
  Serial.println(previousState);

  if (currentState == "open lock")
  {
    lockControl(currentState);
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
  if (content.substring(1) == "14 73 CA 73" || content.substring(1) == "A4 E3 E6 1E" || content.substring(1) == "C6 D9 C8 73") //change UID of the card that you want to give access
  {
    Serial.println("Access Granted");
    delay(500);
    lockControl("open");
    startup = false;
  }

  else
  {
    Serial.println(" Access Denied ");
  }
}

// Fill the dots one after the other with a color, leave the first LED for showing the state
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels() - 1; i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

//LED Switch States
void startShow(int i)
{
  switch (i)
  {
  case 0:
    colorWipe(strip.Color(0, 0, 0), 20); // Black/off
    break;
  case 1:
    colorWipe(strip.Color(0, 200, 18), 20); // Fresa Green
    break;
  case 2:
    colorWipe(strip.Color(0, 0, 0, 255), 20); //White
    break;
  case 3:
    colorWipe(strip.Color(0, 0, 255), 20); // Blue
    break;

  case 4:
    strip.setPixelColor(0, 0, 200, 18); //Single Fresa
    break;
  case 5:
    strip.setPixelColor(0, 255, 0, 0); //Single Red
    break;
  case 6:
    strip.setPixelColor(0, 0, 0, 255); //Single Blue
    break;
  case 7:
    strip.setPixelColor(0, 40, 0, 40); //Single Violet
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
  // strip.show(); // Initialize all pixels to 'off'
  startShow(1);
  delay(1000);
  startShow(0);
  startup = true;
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

  if (lock_state == LOW && !sent && previousState == "reserved" && currentState == "open lock")
  {
    // Serial.println("LOW");
    // send OPEN message to mqtt server
    mqttMsg("loaded");
    startShow(0);
  }
  else if (lock_state == LOW && !sent && currentState == "open" && (previousState == "loaded" || previousState == "open lock" || previousState == "empty loaded") && !startup)
  {
    mqttMsg("empty loaded");
    startShow(0);
  }
  else if (lock_state == LOW && !sent && (currentState == "loaded" || currentState == "empty loaded" || currentState == "open lock"))
  {
    mqttMsg("open");
    startShow(0);
  }
  else if (lock_state == LOW && !sent && currentState == "empty loaded" && previousState == "open")
  {
    mqttMsg("open");
    startShow(0);
  }
}