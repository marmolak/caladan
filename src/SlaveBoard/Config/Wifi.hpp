#pragma once

// This file is ignored by .gitignore.

namespace Config { namespace Wifi {

    const char *const mdns_hostname = "caladan";

    const char *const ssid     = "";
    const char *const password = "";

    // Don't forget to change this passowrd in platformio.ini. 
    const char *const ota_password = "123";

}} // end of namespace