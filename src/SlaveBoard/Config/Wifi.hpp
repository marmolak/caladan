#pragma once

// This file is ignored by .gitignore.

namespace Config { namespace Wifi {

    const char *const mdns_hostname PROGMEM = "caladan";

    const char *const ssid PROGMEM     = "";
    const char *const password PROGMEM = "";

    // Don't forget to change this password in platformio.ini. 
    const char *const ota_password PROGMEM = "123";

}} // end of namespace