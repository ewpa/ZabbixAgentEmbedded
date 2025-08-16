/*
  fullagent.ino  Embedded Arduino Zabbix Agent:  background agent example.

  * Created 15th August 2025.
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

void zabbixActiveTask(void *pvParameter)
{
  Serial.println("Zabbix active task started");
  await_wifi(); // Wait for IP address.
  while (1)
  {
    Serial.println("Send auto-registration");
    send_zabbix_autoreg(ZABBIX_SERVER_ACTIVE_HOST, ZABBIX_SERVER_ACTIVE_PORT, WiFi.localIP(), WiFi.getHostname(), "esp32,arduino,arduino-ex");
    vTaskDelay(60000 / portTICK_PERIOD_MS);
  }
}

int zabbixPassiveItemUpdated(const String key, String &new_value)
{
  Serial.println("Item update request");
  Serial.print(key);
  Serial.print(" -> ");

  int success = 1;
  if (key.equals("system.uptime"))
    new_value = String(millis()/1000);
  else if (key.equals("agent.ping"))
    new_value = "1";
  else if (key.equals("system.hostname"))
    new_value = WiFi.getHostname();
  else if (key.equals("agent.version"))
    new_value = String(ZBX_VERSION_MAJOR) + "." + String(ZBX_VERSION_MINOR)
      + "." + String(ZBX_VERSION_PATCH);
  else
  {
    new_value = "Unsupported item key.";
    success = 0;
  }
  Serial.print(new_value);
  Serial.print(" success=");
  Serial.println(success);
  return success;
}

void zabbixPassiveTask(void *pvParameter)
{
  Serial.println("Zabbix passive task started");
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  zabbix_agent_begin(zabbixPassiveItemUpdated);
  while (1)
  {
    zabbix_agent_loop();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

void loop() {
  // Put your normal loop code here.
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Embedded Arduino Zabbix Agent:  background agent example.");
  Serial.println("Copyright (C) 2025 Ewan Parker");
  Serial.print("Version "); Serial.print(ZBX_VERSION_MAJOR);
  Serial.print("."); Serial.print(ZBX_VERSION_MINOR);
  Serial.print("."); Serial.println(ZBX_VERSION_PATCH);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PSK);

  // Start the Zabbix active task for periodic auto-registration.
  xTaskCreatePinnedToCore(zabbixActiveTask, "zbxa", 3072, NULL,
    (tskIDLE_PRIORITY + 1), NULL, portNUM_PROCESSORS - 1);
  // Start the Zabbix passive task to return item values.
  xTaskCreatePinnedToCore(zabbixPassiveTask, "zbxp", 3072, NULL,
    (tskIDLE_PRIORITY + 1), NULL, portNUM_PROCESSORS - 1);

  // Put your normal setup code here.
}
