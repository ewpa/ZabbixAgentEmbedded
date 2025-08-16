/*
  zbxagent.h  Embedded Arduino Zabbix Agent.

  * Created 3rd August 2025.
  * Copyright (C) 2025 Ewan Parker.
*/

#pragma once

#define ZBX_VERSION_MAJOR 1
#define ZBX_VERSION_MINOR 0
#define ZBX_VERSION_PATCH 1

//#define DEBUG_ZBX 1
#ifndef ZBX_JSON_BUFF_SIZE
#define ZBX_JSON_BUFF_SIZE 256
#endif

#include <Arduino.h>
#include <stdint.h>

void send_zabbix_autoreg(String zabbix_server, uint16_t zabbix_port, IPAddress ip, String host, String host_metadata);
void zabbix_agent_begin(int (*cb)(const String key, String &new_value));
void zabbix_agent_loop();
