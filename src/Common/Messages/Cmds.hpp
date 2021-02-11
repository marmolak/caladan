#pragma once

#include <Arduino.h>
#include <MsgPacketizer.h>

namespace Messages {

enum class Codes : uint8_t
{
    LOG = 0x66,
    CMD = 0x67,
};

enum class CmdOps : uint8_t
{
    INDEX = 0,
    AUTO  = 1,
    START = 2,
    STOP  = 3,
    FORCE = 4,
};


struct MessageLog
{
    static const uint8_t code = static_cast<uint8_t>(Messages::Codes::LOG);
    String content;

    MSGPACK_DEFINE(content);
};

struct MessageCmd
{
    static const uint8_t code = static_cast<uint8_t>(Messages::Codes::CMD);
    uint8_t cmd_op;
    MSGPACK_DEFINE(cmd_op);
};

} // end of namespace

