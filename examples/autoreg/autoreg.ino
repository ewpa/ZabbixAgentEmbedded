/*
  autoreg.ino  Embedded Arduino Zabbix Agent:  auto-registration example.

  * Created 3rd August 2025.
  * Copyright (C) 2025 Ewan Parker.
*/

#include "exconfig.h"
#include "zbxagent.h"
#include <WiFi.h>

void await_wifi()
{
  unsigned long tgt = millis() + 15000;
  while (WiFi.status() != WL_CONNECTED && millis() <= tgt) delay(10);
  if (millis() < tgt)
  {
    Serial.print("WiFi connected: ");
    Serial.print(15000L + millis() - tgt);
    Serial.println("ms");
  }
  else Serial.print("WiFi timeout 15000ms");
}

void loop() {
  // Auto registration.
  Serial.println("Send auto-registration");
  send_zabbix_autoreg(ZABBIX_SERVER_ACTIVE_HOST, ZABBIX_SERVER_ACTIVE_PORT, WiFi.localIP(), WiFi.getHostname(), "esp32,arduino,arduino-ex");

  // Wait 2 minutes.
  delay(120000);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Embedded Arduino Zabbix Agent:  auto-registration example.");
  Serial.println("Copyright (C) 2025 Ewan Parker");
  Serial.print("Version "); Serial.print(ZBX_VERSION_MAJOR);
  Serial.print("."); Serial.print(ZBX_VERSION_MINOR);
  Serial.print("."); Serial.println(ZBX_VERSION_PATCH);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PSK);
  await_wifi();
}
