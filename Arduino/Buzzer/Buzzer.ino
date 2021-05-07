// #define buzzer 12
const int buzzer = 2;
void setup (){
    Serial.begin(115200);
    Serial.setTimeout(1);
    pinMode(buzzer, OUTPUT);
}
void loop () {
    tone(buzzer, 1000); // Send 1KHz sound signal...
    // analoglWrite(buzzer, HIGH);
    Serial.println("Buzzing...");
    delay(2000);        // ...for 1 sec
    noTone(buzzer);     // Stop sound...
    // digitalWrite(buzzer, LOW);
    Serial.println("Silencing...");
    delay(2000);        // ...for 1sec
}