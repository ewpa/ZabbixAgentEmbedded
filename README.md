Zabbix Agent on Arduino ESP32
=============================
An original, simple and effective Zabbix Agent for monitoring IoT devices.  Tested with Zabbix 7.4.

Features
--------
  - Auto-registration in the Zabbix server (using an auto-registration action).
  - Simple and familiar setup plus loop design pattern.
  - Easy to create and send passive item data to Zabbix.

Limitations
-----------
  - Does not perform active checks.
  - PlatformIO support not added or tested (yet).

Example Zabbix Server configuration
-----------------------------------

### Create a new host group.
  1. Navigate: _Data collection >> Host groups_
  2. Choose _Create host group_.
  3. Name the new group _arduino-ex_.
  4. Choose _Add_.

### Import pre-configured template
1. Navigate: _Data collection >> Templates_.
2. Choose _Import_.
3. Browse and select the file _extras/arduino-ex-template.yaml_.
4. Choose _Import_.
5. Review the template and choose _Import_ again.

### Create auto-registration action
  1. Navigate: _Data collection >> Alerts >> Actions >> Autoregistration actions_.
  2. Choose _Create action_.
  3. Name: _Create Arduino host_.
  4. Conditions, Add: _Host metadata contains arduino-ex_, choose _Add_.
  5. Operations, Operations, Add: _Add host_, choose _Add_.
  6. Add: _Add to host group, arduino-ex_, choose _Add_.
  7.   Add: _Link template, arduino-ex, choose _Add_.
  8. Choose _Add_ to add the new action.

Example auto-registration --- autoreg
-------------------------------------
  1. Change the file _examples/autoreg/exconfig.h_ to configure your WiFi and your Zabbix server.
  2. Upload the sketch.
  3. Wait a minute or two.
  4. In Zabbix, navigate: _Monitoring >> Hosts_.
  5. Your host should have been created automatically with a name such as _esp32c3-ABCDEF_.
  6. You will also notice problems being raised for this host.  This is due to no passive checks having a response.  We will fix that in the next example.

Example passive checks --- passivechecks
----------------------------------------
  1. Change the file _examples/passivechecks/exconfig.h_ to configure your WiFi and your Zabbix server.
  2. Upload the sketch to the same board as earlier.
  3. In Zabbix, navigate: _Monitoring >> Hosts_.
  4.  Choose the correct host, e.g. _esp32c3-ABCDEF_, then choose _Latest data_ option from the pop-up.
  5. In about a minute you should see item data being populated in Zabbix.
  6. Try shutting down the Arduino or restarting it.  You will see problems being raised in Zabbix.

Example background agent --- fullagent
--------------------------------------
  1. Change the file _examples/fullagent/exconfig.h_ to configure your WiFi and your Zabbix server.
  2. Upload the sketch.
  3. Wait a minute or two.
  4. The Arduino will appear in the Zabbix server host list, e.g. _esp32c3-ABCDEF_, and items will be populated.
  5. If the host is deleted in Zabbix then it will automatically re-register.
  6. Normal Arduino code can be added to `setup()` and `loop()` as normal.  The agent will run quietly in the background.

See also
--------
  - [Composing the Zabbix header.](https://www.zabbix.com/documentation/current/en/manual/appendix/protocols/header_datalen)
  - [Some JSON agent communication examples](https://www.zabbix.com/documentation/current/en/manual/appendix/items/activepassive)
