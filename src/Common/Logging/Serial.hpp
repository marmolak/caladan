#pragma once

#define SLOG(text) \
do { \
    Serial.print(__FILE__); \
    Serial.print(": "); \
    Serial.print(__func__); \
    Serial.println(": " text); \
} while (false)

