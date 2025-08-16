/*
  passivechecks.ino  Embedded Arduino Zabbix Agent:  passive check example.

  * Created 3rd August 2025.
  * Copyright (C) 2025 Ewan Parker.
*/

#include "exconfig.h"
#include "zbxagent.h"
#include <WiFi.h>

int zabbixPassiveItemUpdated(const String key, String &new_value)
{
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

  Serial.print("Item update request, success=");
  Serial.println(success);
  Serial.print(key);
  Serial.print(" -> ");
  Serial.print(new_value);
  return success;
}

void loop() {
  zabbix_agent_loop();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Embedded Arduino Zabbix Agent:  passive check example.");
  Serial.println("Copyright (C) 2025 Ewan Parker");
  Serial.print("Version "); Serial.print(ZBX_VERSION_MAJOR);
  Serial.print("."); Serial.print(ZBX_VERSION_MINOR);
  Serial.print("."); Serial.println(ZBX_VERSION_PATCH);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PSK);
  zabbix_agent_begin(zabbixPassiveItemUpdated);
  Serial.println("Passive agent started");
}
