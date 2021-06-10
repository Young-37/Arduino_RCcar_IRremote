/*
 * key - map
 * FFC23D ▶|| : 주행 시작 / 종료
 * FFE01F -  : 속도내림
 * FFA857 +  : 속도올림
 * FF22DD |◀◀  : 속도 최하
 * FF02FD ▶▶|  : 숙도 최상
 * FF18E7 2 : 전진
 * FF4AB5 8 : 후진
 * FF5AA5 6 : 우회전
 * FF10EF 4 : 좌회전
 * FF38C7 5 : 정지 / 전진
 * FF906F EQ : 세레머니 (댄스)
 */
/*
    FF30CF 1
    FF18E7 2
    FF7A85 3
    FF10EF 4
    FF38C7 5
    FF5AA5 6
    FF42BD 7
    FF4AB5 8
    FF52AD 9
    FF6897 0
    FF9867 100+
    FFB04F 200+
    FFE01F -
    FFA857 +
    FF906F EQ
    FF22DD |◀◀
    FF02FD ▶▶|
    FFC23D ▶||
    FFA25D CH-
    FF629D CH
    FFE21D CH+
    */
    
#include <IRremote.h>
#include <AFMotor.h>

#define STOPOFF 0
#define FOR 1
#define BACK 2
#define RIGHT 3
#define LEFT 4
#define STOP 5
#define DANCE 6

// IR remote
int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;

// 초음파센서 출력핀(trig)과 입력핀(echo), 변수, 함수 선언//
int TrigPin = A0;
int EchoPin = A1;
long duration, distance;

// DC motor
AF_DCMotor motor_L(3);
AF_DCMotor motor_R(4);

// LED
int LED_PIN = A4;

int motor_speed = 0;
int speed_gap = 30;
int driving_state = 0;
int i = 0;

// 거리 측정
void Obstacle_Check();
void Distance_Measurement();

void Stop_turnOff();
void Stop(int x);
void Forward();
void Backward(int x);
void Right(int x);
void Left(int x);
void Dance();

void IncreaseSpeed();
void DecreaseSpeed();
void SetHighestSpeed();
void SetLowestSpeed();

void setup() {
  Serial.begin(9600);

  // IR remote
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  Serial.println("Start Young's RC car!");

  // 초음파 센서
  pinMode(EchoPin, INPUT);   // EchoPin 입력
  pinMode(TrigPin, OUTPUT);  // TrigPin 출력

  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // motor
  motor_L.run(FORWARD);
  motor_R.run(FORWARD);
  motor_L.setSpeed(motor_speed);
  motor_R.setSpeed(motor_speed);
}

void loop() {
  // LED 제어
  if(driving_state > STOPOFF)  digitalWrite(LED_PIN, HIGH);
  else  digitalWrite(LED_PIN, LOW);
  
  // 리모컨 신호에 따라
  if(irrecv.decode(&results)) {
    Serial.println(results.value, HEX);

    // 주행 시작 || 주행 종료
    if(results.value == 0xFFC23D) {
      if(driving_state == STOPOFF) Forward();  // FFC23D ▶|| : 전진 (주행 시작)
      else Stop_turnOff();  // FFC23D ▶|| : 정지 (주행 종료)
    }

    // 주행 종료 상태가 아닐 때
    if(driving_state > STOPOFF) {
      // 속도 조절
      if(results.value == 0xFFE01F) DecreaseSpeed();  // FFE01F -  : 속도내림
      else if(results.value == 0xFFA857)  IncreaseSpeed();  // FFA857 +  : 속도올림
      else if(results.value == 0xFF22DD)  SetLowestSpeed(); // FF22DD |◀◀  : 속도 최하
      else if(results.value == 0xFF02FD)  SetHighestSpeed(); // FF02FD ▶▶|  : 속도 최상

      // 방향
      else if(results.value == 0xFF4AB5)  Backward(1);  // FF4AB5 8 : 후진
      else if(results.value == 0xFF18E7)  Forward();  // FF18E7 2 : 전진
      else if(results.value == 0xFF5AA5)  Right(1);  // FF5AA5 6 : 우회전
      else if(results.value == 0xFF10EF)  Left(1);  // FF10EF 4 : 좌회전
      else if(results.value == 0xFF38C7) {
        if(driving_state == STOP) Forward();  // FF38C7 5 : 전진
        else if(driving_state != STOPOFF) Stop(1);  // FF38C7 5 : 정지
      }
      else if(results.value == 0xFF906F) Dance(); // FF906F EQ : 세레모니 (댄스)
    }

    // 리모컨 신호 받기
    irrecv.resume(); // Receive the next value
  }
  delay(100);
  
  if(driving_state > STOPOFF)  Obstacle_Check();
}

///////////장애물 확인 및 회피 방향 결정///////////
void Obstacle_Check() {
  int val = random(2);
  Distance_Measurement();

  Serial.println(distance);

  while (distance < 200) {
    if (distance < 180) {
      Backward(0);
      delay(250);
      Stop(0);
      delay(50);
      Distance_Measurement();
    }
    else {
      if (val == 0) {
        Right(0);
        delay(400);
      }
      else if (val == 1) {
        Left(0);
        delay(400);
      }
      Distance_Measurement();
    }
  }

  if(driving_state == FOR)  Forward();
  else if(driving_state == BACK)  Backward(1);
  else if(driving_state == RIGHT) Right(1);
  else if(driving_state == LEFT)  Left(1);
  else if(driving_state == STOP)  Stop(1);
  else if(driving_state == DANCE) Dance();
}


////////거리감지///////////
void Distance_Measurement() {
  digitalWrite(TrigPin, LOW);
  delay(2);
  digitalWrite(TrigPin, HIGH);  // trigPin에서 초음파 발생(echoPin도 HIGH)
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  duration = pulseIn(EchoPin, HIGH);    // echoPin 이 HIGH를 유지한 시간을 저장 한다.
  distance = ((float)(340 * duration) / 1000) / 2;
  delay(5);
}

void Stop_turnOff() {
  motor_L.run(RELEASE);       motor_R.run(RELEASE);
  for (i = motor_speed; i >= 0; i = i - speed_gap) {
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
  motor_speed = i;
  driving_state = STOPOFF;
}

void Stop(int x) {
  motor_L.run(RELEASE);       motor_R.run(RELEASE);
  for (i = motor_speed; i >= 0; i = i - speed_gap) {
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
  motor_speed = i;
  if(x) driving_state = STOP;
}

void Forward() {
  motor_L.run(FORWARD);  motor_R.run(FORWARD);
  if (motor_speed < 165) motor_speed = 200;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
  driving_state = FOR;
}

void Backward(int x) {
  motor_L.run(BACKWARD);  motor_R.run(BACKWARD);
  if (motor_speed < 165) motor_speed = 200;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
  if(x) driving_state = BACK;
}

void Right(int x) {
  if (motor_speed < 225) motor_speed = 225;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
  motor_L.run(FORWARD);  motor_R.run(BACKWARD);
  if(x) driving_state = RIGHT;
  
  for (i = 0; i < 200; i = i + 20) {
    //j = i*1.3;     if(j >= 200) j = 200;
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
  for (i = 180; i < 0; i = i - 20) {
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
}

void Left(int x) {
  if (motor_speed < 225) motor_speed = 225;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
  motor_L.run(BACKWARD);  motor_R.run(FORWARD);
  if(x) driving_state = LEFT;
  
  for (i = 0; i < 200; i = i + 20) {
    //j = i*1.3;     if(j >= 200) j = 200;
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
  for (i = 180; i < 0; i = i - 20) {
    motor_L.setSpeed(i);  motor_R.setSpeed(i);
    delay(2);
  }
}

void Dance() {
  motor_speed = 225;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
  motor_L.run(FORWARD);  motor_R.run(BACKWARD);
  delay(500);
  motor_L.run(BACKWARD);  motor_R.run(FORWARD);
  delay(500);
  driving_state = DANCE;
}

void IncreaseSpeed() {
  motor_speed += speed_gap;
  if(motor_speed > 255) motor_speed = 255;
  else if(motor_speed < 165) motor_speed = 165;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
}

void DecreaseSpeed() {
  motor_speed -= speed_gap;
  if(motor_speed < 165) motor_speed = 165;
  motor_L.setSpeed(motor_speed);  motor_R.setSpeed(motor_speed);
}

void SetHighestSpeed() {
  motor_speed = 255;
  motor_L.setSpeed(motor_speed); motor_R.setSpeed(motor_speed);
}

void SetLowestSpeed() {
  motor_speed = 165;
  motor_L.setSpeed(motor_speed); motor_R.setSpeed(motor_speed);
}
