#include <ESP32Servo.h>

Servo rc1;
Servo rc2;

// ===== PINS =====
const int RC1_PIN = 16;
const int RC2_PIN = 17;

// ===== PWM SETTINGS =====
const int PWM_NEUTRAL = 1500;
const int PWM_FORWARD = 1700;
const int PWM_REVERSE = 1300;

// ===== WRITE BOTH MOTORS =====
void setPWM(int leftPWM, int rightPWM) {
  rc1.writeMicroseconds(leftPWM);
  rc2.writeMicroseconds(rightPWM);

  Serial.print("Left PWM: ");
  Serial.print(leftPWM);

  Serial.print(" | Right PWM: ");
  Serial.println(rightPWM);
}

void setup() {
  Serial.begin(115200);

  rc1.attach(RC1_PIN);
  rc2.attach(RC2_PIN);

  Serial.println("ESC/Motor Test Starting...");

  // ===== ARM ESCs =====
  Serial.println("Sending neutral signal...");
  setPWM(PWM_NEUTRAL, PWM_NEUTRAL);

  delay(3000); // allow ESCs to arm
}

void loop() {

  // ===== FORWARD TEST =====
  Serial.println("FORWARD");

  for (int pwm = PWM_NEUTRAL; pwm <= PWM_FORWARD; pwm += 10) {
    setPWM(pwm, pwm);
    delay(100);
  }

  delay(2000);

  // ===== BACK TO NEUTRAL =====
  Serial.println("NEUTRAL");

  for (int pwm = PWM_FORWARD; pwm >= PWM_NEUTRAL; pwm -= 10) {
    setPWM(pwm, pwm);
    delay(100);
  }

  delay(2000);

  // ===== REVERSE TEST =====
  Serial.println("REVERSE");

  for (int pwm = PWM_NEUTRAL; pwm >= PWM_REVERSE; pwm -= 10) {
    setPWM(pwm, pwm);
    delay(100);
  }

  delay(2000);

  // ===== BACK TO NEUTRAL =====
  Serial.println("NEUTRAL");

  for (int pwm = PWM_REVERSE; pwm <= PWM_NEUTRAL; pwm += 10) {
    setPWM(pwm, pwm);
    delay(100);
  }

  delay(3000);
}