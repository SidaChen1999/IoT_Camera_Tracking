#include <WiFiNINA.h> 
#include <PubSubClient.h>
#include "TXcredentials.h"
#include <ArduinoJson.h>
#include <Servo.h>
#include <FastLED.h>

#define LED_PIN     3
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100
int motorPin = 11;      // LED connected to digital pin 9
int val = 50;         // variable to store the read value
int Rot_1 = 4;
int Rot_2 = 5;
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


const int buzzer = 2;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
StaticJsonDocument<200> doc;

long lastMsg = 0;
long lastAlarm = 0;
String msgStr = "";
char buffer[80];
int x = 0;
int y = 0;
int yaw_ang, pit_ang;
Servo yaw;
Servo pit;
bool update = false;
bool alarm = false;
bool state = false;

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1);
  yaw.attach(9);
  pit.attach(8);
  yaw.write(90);
  pit.write(20);
  pinMode(buzzer, OUTPUT);
  pinMode(motorPin, OUTPUT);  // sets the pin as output
  pinMode(Rot_1, OUTPUT);  // sets the pin as output
  pinMode(Rot_2, OUTPUT);  // sets the pin as output
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  // Motor_init();
  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
  receive();
  receive();
  gimbal_move_both();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  publish();
  buzz();
  ChangePalettePeriodically();
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */  
  FillLEDsFromPaletteColors( startIndex);
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void receive(){
  if (Serial.available()){
    String inBuffer = Serial.readString();
    if (inBuffer[0] == 'x'){
      x = inBuffer.substring(1).toInt();
      float x_float = lround(((float)x / 256 - 0.5) * 47);
      x = (int)x_float;
    }
    else if (inBuffer[0] == 'y'){
      y = inBuffer.substring(1).toInt();
      float y_float = lround(((float)y / 256 - 0.5) * 31);
      y = (int)y_float;
      update = true;
    }
  }
}

void print2python(){
  sprintf(buffer,"(%d,%d)",x,y);
  Serial.print(buffer);
  alarm = true;
  lastAlarm = millis();
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    // Serial.print(".");
    WiFi.begin(ssid, password);
  }
  randomSeed(micros());
}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  while (!client.connected()) {
    String clientId = "ArduinoClient-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
      client.subscribe(subTopic);
    } 
    else {
      delay(1000);
    }
  }
}

void publish(){
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    if (alarm) {
      msgStr=msgStr+"field1="+1;
    }
    else {
      msgStr=msgStr+"field1="+2;
    }
    byte arrSize = msgStr.length() + 1;
    char msg[arrSize];
    msgStr.toCharArray(msg, arrSize);
    client.publish(pubTopic, msg);
    msgStr = "";
  }
}

long lastTime = 0;
void buzz(){
  long timer = millis();
  if (alarm && state) {
    if ((timer - lastAlarm) % 2000 == 0) {
      tone(buzzer, 1000);
      digitalWrite(Rot_1, HIGH);
      digitalWrite(Rot_2, LOW);
      analogWrite(motorPin, 80);
    }
    else if ((timer - lastAlarm) % 2000 == 1000) {
      tone(buzzer, 200);
      digitalWrite(Rot_1, LOW);
      digitalWrite(Rot_2, HIGH);
      analogWrite(motorPin, 80);
    }
    // else if ((timer - lastAlarm) % 4000 == 200) {
    //   digitalWrite(Rot_1, HIGH);
    //   digitalWrite(Rot_2, LOW);
    //   analogWrite(motorPin, 100);
    // }
    // else if ((timer - lastAlarm) % 4000 == 2200){
    //   digitalWrite(Rot_1, LOW);
    //   digitalWrite(Rot_2, HIGH);
    //   analogWrite(motorPin, 100);
    // }
  }
  else {
    noTone(buzzer);
    motorStop();
  }
  if (timer - lastAlarm > 30000) {
    alarm = false; 
  }
}

char recv_buffer[1024];
char temp;
void callback(char* topic, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) 
  {
    temp = (char)payload[i];
    // Serial.print((char)payload[i]);
    recv_buffer[i] = temp;
  }
  recv_buffer[length] = '\0';

  parseJson();
}

void parseJson(){
  DeserializationError error = deserializeJson(doc, recv_buffer);
  if (error) {
    // Serial.print(F("deserializeJson() failed: "));
    // Serial.println(error.f_str());
    return;
  }
  int status = doc["field2"];

  if (status == 1) {
    state = true;
  }
  else if (status == 2) {
    state = false;
  }
}

void gimbal_move_both() {
  if(update == true){
    yaw.write(yaw.read() - x);
    pit.write(pit.read() - y);
    print2python();
    update = false;
    x = 0;
    y = 0;
  };
}

void Motor_init(){
  analogWrite(motorPin,150);
  digitalWrite(Rot_1,1);
  digitalWrite(Rot_2,0);
  delay(500);
}

void motorStop() {
  digitalWrite(Rot_1,HIGH);
  digitalWrite(Rot_2, LOW);
  analogWrite(motorPin, 0);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}


// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};

