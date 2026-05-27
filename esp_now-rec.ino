#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

typedef struct {
  int16_t throttle;  // -100 (full reverse) to 100 (full forward)
  int16_t steering;  // -100 (full left) to 100 (full right)
} DrivePacket;

DrivePacket data;

Servo motorESC;
Servo steeringServo;

// ===== HARDWARE PINS =====
const int MOTOR_PIN = 17;
const int STEERING_PIN = 18;

const int PWM_NEUTRAL = 1500;
const int PWM_RANGE = 500;

// ===== speed → pulse =====
int speedToPWM(int speed) {
  speed = constrain(speed, -100, 100);
  return PWM_NEUTRAL + (speed * PWM_RANGE / 100);
}

// ===== car control =====
void driveCar(int16_t throttle, int16_t steering) {
  int pwm_motor = speedToPWM(throttle);
  
  // Map steering (-100 to 100) to servo angle (40 to 140)
  int steering_val = constrain(steering, -100, 100);
  int steer_angle = map(steering_val, -100, 100, 40, 140);

  Serial.print("Throttle: "); Serial.print(throttle);
  Serial.print(" -> PWM: "); Serial.print(pwm_motor);
  Serial.print(" | Steering: "); Serial.print(steering);
  Serial.print(" -> Angle: "); Serial.println(steer_angle);

  motorESC.writeMicroseconds(pwm_motor);
  steeringServo.write(steer_angle);
}

// ===== ESP-NOW CALLBACK =====
void OnDataRecv(const esp_now_recv_info *info,
                const uint8_t *incomingData,
                int len) {

  memcpy(&data, incomingData, sizeof(data));
  driveCar(data.throttle, data.steering);
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

  // ===== SERVO & ESC INITIALIZATION =====
  motorESC.attach(MOTOR_PIN);
  steeringServo.attach(STEERING_PIN);

  // Set to neutral on startup
  driveCar(0, 0);

  Serial.println("Receiver Ready (Servo Steering & Motor Driver)");
}

void loop() {}