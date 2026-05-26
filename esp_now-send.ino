#include <esp_now.h>
#include <WiFi.h>
//1C:C3:AB:C1:3D:EC
uint8_t robotMAC[] = {0x1C,0xC3,0xAB,0xC1,0x3D,0xEC};

typedef struct {
  int16_t left_speed;
  int16_t right_speed;
} DrivePacket;

DrivePacket data;

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  memcpy(peerInfo.peer_addr, robotMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  Serial.println("Send: left right (e.g. 50 50)");
}

void loop() {

  if (Serial.available()) {

    String msg = Serial.readStringUntil('\n');

    int spaceIndex = msg.indexOf(' ');

    if (spaceIndex == -1) return;

    int left = msg.substring(0, spaceIndex).toInt();
    int right = msg.substring(spaceIndex + 1).toInt();

    data.left_speed = left;
    data.right_speed = right;

    esp_now_send(robotMAC, (uint8_t*)&data, sizeof(data));

    Serial.print("Sent L=");
    Serial.print(left);
    Serial.print(" R=");
    Serial.println(right);
  }
}