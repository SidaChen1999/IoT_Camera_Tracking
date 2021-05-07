#include <TimerOne.h>
int pwm1 = 1; //en1
int in1A = 4;
int in1B = 5;
int encA1 = 7;
int encB1 = 6;

double v1_real = 0;
double v1 = 0;
float kp = 0.005, ki = 0.00005, kd = 0;
int maxV = 127, minV = -127;
double maxIntegral = 1000000, minIntegral = -1000000;
char c='S';
double sp = 0;
float ltime=0;
float ntime=0;
// double pi=3.1415926535;
double ktime=0;

void setup () {
  pinMode(pwm1, OUTPUT);
  pinMode(in1A, OUTPUT);
  pinMode(in1B, OUTPUT);
  pinMode(encA1, INPUT);
  pinMode(encB1, INPUT);
  Serial.begin(115200);
  attachInterrupt(encA1, counter1, RISING);
  Timer1.initialize(10000);
  Timer1.attachInterrupt( timerIsr );
  interrupts();
}

int timer1 = 0;
void loop () {
  if(Serial.available()){
    char ch=Serial.read();
    // if(ch=='d'){
    //   angle=Serial.parseFloat();
    //   Serial.println(angle);
    // }
    if(ch=='s'){
      sp=Serial.parseFloat();
      Serial.println(sp);
      if(sp!=0){
        ktime=millis();
      }      
    }
    // else if(ch=='U'||ch=='D'){
    //   c=ch;
    //   ltime=millis();
    //   Serial.println(c);
    // }
    // else if(ch=='S'){
    //   c=ch;
    //   Serial.println(c);
    // }
  }
  char l[40];
  sprintf(l,"V1 target: %.2f; current: %.2f; pwm: %.2f; integral: %.2f", v1, v1_real, output_pwm1, integral1);
  Serial.println(l);

  if (millis() - timer1 > 10){
    timer1 = millis();
    myPID1();
    motor(pwm1, in1B, in1A, output_pwm1);
  }
}


double integral1 = 0, integral_pre1 = 0, err1 = 0, err_pre1 = 0;
double output_pwm1 = 0;
int t1 = 0, t_pre1 = 0;
double dt1 = 0;
void myPID1(){
  t1 = millis();
  dt1 = t1 - t_pre1;
  err1 = v1 - v1_real;
  integral1 = integral_pre1 + dt1 * err1;
  output_pwm1 = kp * err1 + ki * integral1;
  //+ (kd * (err - err_pre) / dt);
  if(output_pwm1 > maxV){
    output_pwm1 = maxV;
    integral1 = integral_pre1;
  }
  if(output_pwm1 < minV){
    output_pwm1 = minV;
    integral1 = integral_pre1;
  }
  
  if(integral1 > maxIntegral){
    integral1 = integral_pre1;
  }
  if(integral1 < minIntegral){
    integral1 = integral_pre1;
  }

  t_pre1 = t1;
  integral_pre1 = integral1;
  err_pre1 = err1;
}

int count1 = 0;
void counter1()
{
  if(digitalRead(encB1)==LOW){
    count1++;
  }
  else{
    count1--;
  }
}

int j = 0;
void timerIsr()
{
   j = 1;     
   v1_real = 100 * (double)count1;
   count1 = 0;
   /*
   Serial.print(v3_real);
   Serial.print(" ");
   Serial.print(v3);
   Serial.print(" ");
   Serial.println(output_pwm3);
   */
}

void motor(int pwm0, int inA,int inB, double v){
  // if((v > 0 && (pwm0 == 6 || pwm0 == 35)) || (v < 0 && (pwm0 == 2 || pwm0 == 30))){
  //   analogWrite(pwm0,abs(v));
  //   digitalWrite(inA,HIGH);
  //   digitalWrite(inB,LOW);
  // }
  // else if((v < 0 && (pwm0 == 6 || pwm0 == 35)) || (v > 0 && (pwm0 == 2 || pwm0 == 30))){
  //   analogWrite(pwm0,abs(v));
  //   digitalWrite(inA,LOW);
  //   digitalWrite(inB,HIGH);
  // }
  // else{
  //   analogWrite(pwm0,v);
  //   digitalWrite(inA,LOW);
  //   digitalWrite(inB,LOW);
  // } 
  if (v > 0) {
    analogWrite(pwm0,abs(v));
    digitalWrite(inA,HIGH);
    digitalWrite(inB,LOW);
  }
  else if (v < 0) {
    analogWrite(pwm0,abs(v));
    digitalWrite(inA,LOW);
    digitalWrite(inB,HIGH);
  }
  else {
    analogWrite(pwm0,v);
    digitalWrite(inA,LOW);
    digitalWrite(inB,LOW);
  }
}
