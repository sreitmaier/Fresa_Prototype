#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <MFRC522.h>

#define PIN 14
#define NUM_LEDS 10
#define BRIGHTNESS 50


constexpr uint8_t RST_PIN = 21;          // Setting for Adafruit Feather ESP32
constexpr uint8_t SS_PIN = 23;         // Setting for Adafruit Feather ESP32

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
int statuss = 0;
int out = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };




void setup() {

  //LED

  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

//RFID
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522


  
}


void loop() {


  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
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
  if (content.substring(1) == "C6 D9 C8 73") //change UID of the card that you want to give access
  {
    Serial.println(" Access Granted ");
    Serial.println(" Hallo Sabine Reitmaier ");
    delay(1000);
    Serial.println(" Nimmm dein Fresa Paket mit!");
    Serial.println();
    statuss = 1;
    startShow(1);
     //light the LEDS Fresa Style
 //colorWipe(strip.Color(0, 255, 0), 20); // Green

  //pulseGreenWhite(7); 
  delay(1000);
  startShow(0);
 
  
  }
  
  else   {
    Serial.println(" Access Denied ");
    delay(3000);
  }

  

}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);

    
    
  }
}



//TestLib for Turning LED off again afer lock closes
void startShow(int i) {
   switch(i){
     case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
             break;
     case 1: colorWipe(strip.Color(0, 255, 0), 20);  // Green
             break;
    case 2: colorWipe(strip.Color(0,0, 0,255), 20);  //White
             break;
   // case 3: colorWipe(strip.Color(0, 255, 0), 20);
            // break;
   }
   strip.show();
}



void pulseGreenWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0, 255, 0, neopix_gamma[j] ) );
        }
        delay(wait);
        strip.show();
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0, 255, 0, neopix_gamma[j] ) );
        }
        delay(wait+ 15);
        strip.show();
      }
}



void fullWhite() {
  
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
    }
      strip.show();
}


  
  //Color Variations
 //colorWipe(strip.Color(0, 0, 0, 255), 50); // White
 //fullWhite();
 //delay(2000);
