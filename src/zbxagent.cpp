/*
  zbxagent.cpp  Embedded Arduino Zabbix Agent.

  * Created 3rd August 2025.
  * Copyright (C) 2025–2026 Ewan Parker.
*/

#include "zbxagent.h"

#if DEBUG_ZBX
#define debug_print Serial.print
#define debug_println Serial.println
#else
#define debug_print(...) {}
#define debug_println(...) {}
#endif

#include <WiFi.h>
#include <NetworkServer.h>
#include <NetworkClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
typedef NetworkServer EthernetServer;
typedef NetworkClient EthernetClient;
#include <string.h>

int (*item_update_cb)(const String key, String &new_value) = NULL;

EthernetServer server(10050);
EthernetClient in_client, out_client;

void compose_zabbix_header(unsigned char buffer[13], int datalen)
{
  bzero(buffer, 13);
  strncpy((char*)buffer, "ZBXD\1", 5);
  buffer[5] = datalen&0xff;
  buffer[6] = (datalen>>8)&0xff;
  buffer[7] = (datalen>>16)&0xff;
  buffer[8] = (datalen>>24)&0xff;
}

int compose_zabbix_data(char buffer[ZBX_JSON_BUFF_SIZE], char *k, String v)
{
  JsonDocument out;
  out["version"] = "7.4.0";
  out["variant"] = 2;
  out["data"][0][k] = v;
  bzero(buffer, ZBX_JSON_BUFF_SIZE);
  char tmp[ZBX_JSON_BUFF_SIZE];
  serializeJson(out, tmp);
  memcpy(buffer, tmp, ZBX_JSON_BUFF_SIZE);
  //serializeJson(out, buffer);
  #if DEBUG_ZBX
  serializeJson(out, Serial);
  debug_println();
  #endif /* DEBUG_ZBX */
  return strnlen(buffer, ZBX_JSON_BUFF_SIZE);
}

int calc_zabbix_datalen(char *k, String v)
{
  char resp[ZBX_JSON_BUFF_SIZE];
  return compose_zabbix_data(resp, k, v);
}

void send_zabbix_response(char *k, String v)
{
  char resp[ZBX_JSON_BUFF_SIZE];
  int datalen = compose_zabbix_data(resp, k, v);
  unsigned char header[13];
  compose_zabbix_header(header, datalen);
  int bytes = in_client.write(header, 13);
  debug_print("Write hdr bytes: ");
  debug_println(bytes);
  in_client.write(resp, datalen);
  debug_print("Data len: ");
  debug_println(datalen);
}

int send_zabbix_active_checks
(const char *zabbix_server, uint16_t zabbix_port, const char *ip, String host,
String host_metadata)
{
  int c = out_client.connect(zabbix_server, zabbix_port);
  if (c)
  {
    debug_println();
    debug_println("Send active checks");
    JsonDocument out;
    out["request"] = "active checks";
    out["ip"] = ip;
    out["host"] = host;
    out["host_metadata"] = host_metadata;
    out["version"] = "7.4.0";
    out["variant"] = 2;
    char buffer[ZBX_JSON_BUFF_SIZE];
    bzero(buffer, ZBX_JSON_BUFF_SIZE);
    serializeJson(out, buffer);
    #if DEBUG_ZBX
    serializeJson(out, Serial);
    debug_println();
    #endif /* DEBUG_ZBX */
    int datalen = strnlen(buffer, ZBX_JSON_BUFF_SIZE);
    unsigned char header[13];
    compose_zabbix_header(header, datalen);
    int bytes = out_client.write(header, 13);
    debug_print("Write hdr bytes: ");
    debug_println(bytes);
    bytes = out_client.write(buffer, datalen);
    debug_print("Data len/written: ");
    debug_print(datalen);
    debug_print("/");
    debug_println(bytes);
    out_client.clear();
    unsigned long tgt = millis() + 3000;
    while (out_client.available() < 13 && millis() <= tgt) delay(10);
    if (millis() < tgt)
    {
      debug_print("Response: ");
      debug_print(3000L + millis() - tgt);
      debug_println("ms");
    }
    else debug_println("Response timeout 3000ms");
    bzero(buffer, ZBX_JSON_BUFF_SIZE);
    out_client.read((unsigned char*)buffer, 13);
    bytes = out_client.read((unsigned char*)buffer, ZBX_JSON_BUFF_SIZE);
    debug_print("Response datalen: ");
    debug_println(bytes);
    debug_println(buffer);
    out_client.stop();
    return 0;
  }
  else
  {
    debug_println("Active checks failed");
    return -1;
  }
}

int send_zabbix_autoreg
(String zabbix_server, uint16_t zabbix_port, IPAddress ip, String host,
String host_metadata)
{
  return send_zabbix_active_checks(
    zabbix_server.c_str(), zabbix_port, ip.toString().c_str(), host,
    host_metadata);
}

void zabbix_agent_begin(int (*cb)(const String key, String &new_value))
{
  item_update_cb = cb;
  server.begin();
  debug_println();
  debug_println("Network server started");
}

void zabbix_agent_loop() {
  in_client = server.available();
  if (in_client) {
    unsigned char header[13];
    debug_println();
    debug_println("Recv passive checks");
    unsigned long tgt = millis() + 3000;
    while (in_client.available() < 13 && millis() <= tgt) delay(10);
    if (millis() < tgt)
    {
      debug_print("Response: ");
      debug_print(3000L + millis() - tgt);
      debug_println("ms");
    }
    else debug_println("Response timeout 3000ms");

    if (in_client.available() >= 13) {
      int bytes = in_client.readBytes(header, 13);
      debug_print("Read hdr bytes: ");
      debug_println(bytes);
      if (!memcmp(header, "ZBXD\1", 5))
      {
        unsigned int datalen
          = header[5]|(header[6]<<8)|(header[7]<<16)|(header[8]<<24);
        debug_print("Data len: ");
        debug_println(datalen);
        JsonDocument in;
        #if DEBUG_ZBX
        unsigned char body[ZBX_JSON_BUFF_SIZE];
        bzero(body, ZBX_JSON_BUFF_SIZE);
        int bytes = in_client.readBytes(body, ZBX_JSON_BUFF_SIZE - 1);
        debug_print("Read req bytes: ");
        debug_println(bytes);
        for (int i = 0; i < bytes; i++) debug_print((char)body[i]);
        debug_println();
        DeserializationError error = deserializeJson(in, body);
        #else /* DEBUG_ZBX */
        DeserializationError error = deserializeJson(in, in_client);
        #endif /* DEBUG_ZBX */
        if (error) {
          debug_print(F("deserializeJson() failed: "));
          debug_println(error.c_str());
        }
        else {
          String req = in["request"];
          String key = in["data"][0]["key"];
          debug_print("Req: "); debug_println(req);
          debug_print("Key: "); debug_println(key);
          if (item_update_cb)
          {
            String newval = "";
            if (item_update_cb(key, newval))
              send_zabbix_response("value", newval);
            else // not supported
              send_zabbix_response("error", newval);
          }
          else
            send_zabbix_response("error", "Agent configuration error.");
        }
      }
      else {
        debug_print("Hdr mismatch, found: ");
        debug_print(header[0], HEX);
        debug_print(header[1], HEX);
        debug_print(header[2], HEX);
        debug_print(header[3], HEX);
        debug_println(header[4], HEX);
      }
    }
    in_client.stop();
  }
}
