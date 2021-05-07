const int switcher = 3;
const int LED = 13;
void setup (){
    Serial.begin(115200);
    pinMode(switcher, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
}
void loop () {
    readState();
}
int state = LOW;
void readState(){
    int state = digitalRead(switcher);
    Serial.println(state);
    if (state == HIGH) {
        digitalWrite(LED, LOW);
    } else {
        digitalWrite(LED, HIGH);
  }
}