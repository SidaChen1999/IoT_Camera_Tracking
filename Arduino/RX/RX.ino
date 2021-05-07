#include <WiFiNINA.h> 
#include <PubSubClient.h>
#include "RXcredentials.h"
#include <ArduinoJson.h>

const int buzzer = 2;
const int switcher = 3;
const int LED = 13;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
long lastMsg = 0;
String msgStr = "";
long lastAlarm = 0;
bool buzzing = false;
bool state = false;
StaticJsonDocument<200> doc;

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    WiFi.begin(ssid, password);
  }
  randomSeed(micros());
  Serial.println("");
  Serial.print("WiFi connected");
  Serial.print("\tIP address: ");
  Serial.println(WiFi.localIP());
}

char recv_buffer[1024];
char temp;
void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    temp = (char)payload[i];
    // Serial.print((char)payload[i]);
    recv_buffer[i] = temp;
  }
  recv_buffer[length] = '\0';
  // Serial.println();
  // Serial.println(recv_buffer);
  parseJson();
}

void reconnect() 
{
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ArduinoClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) 
    {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(subTopic);
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  pinMode(switcher, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void loop() {
  readState();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // long now = millis();
  // if (now - lastMsg > 5000) {
  //   lastMsg = now;
  // }
  publish();
  buzz();
}

void parseJson(){
  DeserializationError error = deserializeJson(doc, recv_buffer);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  int alarm = doc["field1"];
  Serial.print("Received field1: ");
  Serial.println(alarm);
  if (alarm == 1) {
    buzzing = true;
    lastAlarm = millis();
  }
  else if (alarm == 2) {
    buzzing = false;
  }
}

void buzz(){
  long timer = millis();
  if (buzzing && state) {
    if ((timer - lastAlarm) % 2000 == 0) {
      tone(buzzer, 1000);
      // Serial.println("High ");
    }
    else if ((timer - lastAlarm) % 2000 == 1000) {
      tone(buzzer, 100);
      // Serial.println("Low ");
    }
  }
  else {
    noTone(buzzer);
  }
  if (timer - lastAlarm > 30000){
    buzzing = false;
  }
}

int switch_state = LOW;
void readState(){
  int switch_state = digitalRead(switcher);
  // Serial.println(switch_state);
  if (switch_state == HIGH) {
    digitalWrite(LED, LOW);
    state = false;
  } else {
    digitalWrite(LED, HIGH);
    state = true;
  }

}

void publish(){
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    //create the string to be published
    if (state) {
      msgStr=msgStr+"field2="+1;
    }
    else {
      msgStr=msgStr+"field2="+2;
    }
    byte arrSize = msgStr.length() + 1;
    char msg[arrSize];
    Serial.print("Publish message: ");
    Serial.println(msgStr);
    msgStr.toCharArray(msg, arrSize);
    //publish the string to the server
    client.publish(pubTopic, msg);
    msgStr = "";
  }
  // if (now - lastAlarm > 30000) {
  //   alarm = false; 
  // }
}