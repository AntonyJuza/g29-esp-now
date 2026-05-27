#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

typedef struct {
  int16_t left_speed;
  int16_t right_speed;
} DrivePacket;

DrivePacket data;

Servo rc1;
Servo rc2;

// ===== MOTOR PINS =====
const int RC1_PIN = 16;
const int RC2_PIN = 17;

const int PWM_NEUTRAL = 1500;
const int PWM_RANGE = 500;

// ===== speed → pulse =====
int speedToPWM(int speed) {
  speed = constrain(speed, -100, 100);
  return PWM_NEUTRAL + (speed * PWM_RANGE / 100);
}

// ===== motor control =====
void setLeftRight(int16_t left, int16_t right) {
  int pwm_left = speedToPWM(left);
  int pwm_right = speedToPWM(right);

  Serial.print("L: "); Serial.print(left);
  Serial.print(" -> "); Serial.print(pwm_left);
  Serial.print(" | R: "); Serial.print(right);
  Serial.print(" -> "); Serial.println(pwm_right);

  rc1.writeMicroseconds(pwm_left);
  rc2.writeMicroseconds(pwm_right);
}

// ===== ESP-NOW CALLBACK =====
void OnDataRecv(const esp_now_recv_info *info,
                const uint8_t *incomingData,
                int len) {

  memcpy(&data, incomingData, sizeof(data));
  setLeftRight(data.left_speed, data.right_speed);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  // ===== SERVO INITIALIZATION =====
  rc1.attach(RC1_PIN);
  rc2.attach(RC2_PIN);

  setLeftRight(0, 0);

  Serial.println("Receiver Ready (ESP32 Servo)");
}

void loop() {}