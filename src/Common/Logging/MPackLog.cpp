#include <MsgPacketizer.h>

#include "Common/Logging/MPackLog.hpp"
#include "Common/Messages/Cmds.hpp"

void MPackLog::mlog(const char *const file, const char *const func, const String log)
{
    /* fails with exception - need to investigate how String works
    const auto fil = String(file);
    const auto fce = String(func);

    const String clog = fil + ": " + fce + ": " + log;
    */
    const Messages::MessageLog m = {
        .content = log
    };
    MsgPacketizer::send(Serial, Messages::MessageLog::code, m);
}