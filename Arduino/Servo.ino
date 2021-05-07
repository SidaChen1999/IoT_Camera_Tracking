#include <Servo.h>

//x,y.
int x = 0;
int y = 0;
int yaw_ang, pit_ang;
Servo yaw;
Servo pit;
bool update = false;
//void gimbal_move_both();

void setup()
{
  Serial.begin(115200);
  yaw.attach(9);
  pit.attach(8);
  yaw.write(0);
  pit.write(11);
  delay(1000);
}

void loop()
{ 
//  yaw.write(0);
//  delay(2000);
//  yaw.write(45);
//  delay(2000);
//  yaw.write(90);
//  delay(2000);
  
  receive();
  receive();
  gimbal_move_both();
  delay(50);
}

//Function.
void gimbal_move_both() {
  if(update == true){
    yaw.write(yaw.read() - x);
    pit.write(pit.read() - y);
    update = false;
    x = 0;
    y = 0;
  };
}

void receive(){
  if (Serial.available()){
    String inBuffer = Serial.readString();
    if (inBuffer[0] == 'x'){
      x = inBuffer.substring(1).toInt();
      Serial.println(x);
    }
    else if (inBuffer[0] == 'y'){
      y = inBuffer.substring(1).toInt();
      Serial.println(y);
      update = true;
    }
  }
}
