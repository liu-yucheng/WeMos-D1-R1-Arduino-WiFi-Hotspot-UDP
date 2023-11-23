// Copyright (C) 2023 Yucheng Liu, GNU AGPL3 license.

// Developers: Yucheng Liu
// Emails: yliu428@connect.hkust-gz.edu.cn

// WeMos D1 R1 WiFi Hotspot UDP.

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#define SERIAL_BAUD_RATE 115200
#define IN_PACKET_BYTE_COUNT 2048
#define OUT_PACKET_BYTE_COUNT 2048
#define REMOTE_PORTS_COUNT 6
#define WIFI_STATUS_PRINT_INTERVAL_MS 1000

const char ssid[] = "WeMos-D1-R1-WiFi-Hotspot-UDP";
const char password[] = "wemos-d1-r1-wifi-hotspot-udp";

int ledOn = 1;

IPAddress localIP(192, 168, 1, 1);
IPAddress localGateway(192, 168, 1, 1);
IPAddress localSubnet(255, 255, 255, 0);

Ticker wifiStatusPrintTicker;

WiFiUDP localUDP;
unsigned int localPort = 50420;

long packetIndex = 0;
char inPacket[IN_PACKET_BYTE_COUNT];
char outPacket[OUT_PACKET_BYTE_COUNT];
IPAddress remoteIP(192, 168, 1, 255);
unsigned int remotePorts[REMOTE_PORTS_COUNT] = { 10100, 20100, 30100, 40100, 50100, 60100 };

void setup();
void loop();

void setupSerial();
void setupLED();
void setupWiFi();
void setupWiFiStatusPrint();
void setupUDP();
void loopUDP();
void printWiFiStatus();
void toggleLED();

void setup() {
  setupSerial();
  setupLED();
  setupWiFi();
  setupWiFiStatusPrint();
  setupUDP();
}

void loop() {
  loopUDP();
}

void setupSerial() {
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();
  Serial.printf("Completed setting up serial\n");
}

void setupLED() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.printf("Completed setting up LED\n");
}

void setupWiFi() {
  Serial.printf("Will configure WiFi soft AP\n");
  bool received = WiFi.softAPConfig(localIP, localGateway, localSubnet);
  Serial.printf(received ? "Configuration received\n" : "Configuration failed\n");

  Serial.printf("Will configure WiFi soft AP\n");
  // WiFi.softAP(ssid);
  // WiFi.softAP(ssid, password, channel, hidden, max_connection);
  received = WiFi.softAP(ssid, password);
  Serial.printf(received ? "Setup received\n" : "Setup failed\n");

  Serial.printf("WiFi soft AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
  toggleLED();
}

void setupWiFiStatusPrint() {
  wifiStatusPrintTicker.attach_ms(WIFI_STATUS_PRINT_INTERVAL_MS, printWiFiStatus);
  Serial.printf("Completed setting up WiFi status printing\n");
  toggleLED();
}

void setupUDP() {
  localUDP.begin(localPort);
  Serial.printf("Began WiFi UDP; IP: %s; Port: %d\n", WiFi.localIP().toString().c_str(), localPort);
  toggleLED();
}

void loopUDP() {
  unsigned long timeMs = millis();
  Serial.printf("Will attempt to receive data packet; timeMs: %ld\n", timeMs);
  int packetSize = localUDP.parsePacket();
  bool received = packetSize > 0;

  if (received) {
    Serial.printf(
      "Received data packet; Byte count: %d; Sender IP: %s; Sender port: %d\n",
      packetSize,
      localUDP.remoteIP().toString().c_str(),
      localUDP.remotePort());

    int length = localUDP.read(inPacket, IN_PACKET_BYTE_COUNT);

    if (length > 0) {
      inPacket[length] = '\0';
    }

    Serial.printf("\n- Begin - Received packet contents\n\n");
    Serial.printf("%s\n", inPacket);
    Serial.printf("\n- End - Received packet contents\n\n");
  }

  timeMs = millis();
  Serial.printf("Will send data packet; timeMs: %ld\n", timeMs);
  sprintf(outPacket, "packetIndex: %d\n", packetIndex);
  int sent;

  for (int index = 0; index < REMOTE_PORTS_COUNT; index += 1) {
    localUDP.beginPacket(remoteIP, remotePorts[index]);
    localUDP.write((const unsigned char*)outPacket, strlen(outPacket));
    sent = localUDP.endPacket();

    Serial.printf("Sent UDP packet\n");

    Serial.printf(
      "Sent: %s; Receiver IP: %s; Receiver port: %d\n",
      sent ? "true" : "false",
      remoteIP.toString().c_str(),
      remotePorts[index]);
  }

  Serial.printf("\n- Begin - Sent packet contents\n\n");
  Serial.printf("%s\n", outPacket);
  Serial.printf("\n- End - Sent packet contents\n\n");
  Serial.printf("Sent data packet\n");

  packetIndex += 1;
}

void printWiFiStatus() {
  Serial.printf("WiFi soft AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
  toggleLED();
}

void toggleLED() {
  if (ledOn == 0) {
    digitalWrite(LED_BUILTIN, LOW);
    ledOn = 1;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    ledOn = 0;
  }
}
