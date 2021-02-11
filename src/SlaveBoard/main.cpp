#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>

#include <ArduinoOTA.h>

#include "Common/Logging/MPackLog.hpp"
#include "Config/Wifi.hpp"
#include "SlaveBoard/HTTPServer/HTTPServer.hpp"

// Workaround for newest c++14 version
void operator delete(void *ptr, size_t size)
{
    (void) size;
    free(ptr);
}

void operator delete[](void *ptr, size_t size)
{
    (void) size;
    free(ptr);
}

void operator delete[](void *ptr)
{
    free(ptr);
}

namespace {

bool update_in_progress = false;

HTTPServer http_server;

void setup_OTA()
{
    ArduinoOTA.onStart([]()
    {
      update_in_progress = true;

      Serial.flush();

      if (ArduinoOTA.getCommand() == U_FLASH)
      {
        return;
      }

      // Take care only about FS flash.
      LittleFS.end();
    });

    ArduinoOTA.onEnd([]()
    {
        update_in_progress = false;
        LittleFS.begin();
    });
  
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
    //  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
      /*Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");*/
    });


    ArduinoOTA.setHostname((const char *) Config::Wifi::mdns_hostname);
    ArduinoOTA.setPassword((const char *) Config::Wifi::ota_password);

    ArduinoOTA.begin();
}

} // end namespace

void setup()
{
  Serial.begin(9600);

  MLOG("Starting Slave Board.");

  const bool ok = LittleFS.begin();
  if (!ok) {
      MLOG("Unable to init filesystem.");
      delay(5000);
      ESP.restart();
      return;
  }

  MLOG("Wifi init...");
  WiFi.begin(FPSTR(Config::Wifi::ssid), FPSTR(Config::Wifi::password));
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      //ESP.restart();
  }

  MLOG("HTTP init...");
  ::http_server.init();

  MLOG("OTA init...");
  setup_OTA();

  MLOG("MDNS init...");
  MDNS.begin(FPSTR(Config::Wifi::mdns_hostname));
  MDNS.addService("http", "tcp", 80);
}

void loop()
{
    ArduinoOTA.handle();
    if (update_in_progress)
    {
      return;
    }

    MDNS.update();
  
    http_server.handle_requests();

  /*if (started)
  {
    const unsigned long current_time = millis();
    if ((current_time - start_time) >= (1 * 60 * 1000))
    {
      stop_system();
    }
  }

  server.handleClient();*/
}