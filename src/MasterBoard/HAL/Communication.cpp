#include <Arduino.h>
#include <MsgPacketizer.h>

#include "MasterBoard/HAL/Communication.hpp"

MasterBoardComm::queue_t MasterBoardComm::m_queue;

void MasterBoardComm::init()
{
    if (!Serial3)
    {
        Serial3.begin(9600);
    }


    // parse cmd packet
    MsgPacketizer::subscribe(Serial3, Messages::MessageLog::code, [](const Messages::MessageLog &n)
    {
        Serial.println(n.content);
    });

    MsgPacketizer::subscribe(Serial3, Messages::MessageCmd::code, [](const Messages::MessageCmd &n)
    {
        const Messages::CmdOps cmd_op = static_cast<Messages::CmdOps>(n.cmd_op);
        switch (cmd_op)
        {
            case Messages::CmdOps::AUTO:
            case Messages::CmdOps::FORCE:
            case Messages::CmdOps::START:
            case Messages::CmdOps::STOP:
            {
                const bool ok = MasterBoardComm::m_queue.push(cmd_op);
                if (!ok) {
                    Serial.println("Unable to add to queue.");
                    return;
                }
                return;
                break;
            }
            case Messages::CmdOps::INDEX:
            default:
                Serial.println("Message not supported!");
                return;
                break;
        }
    });
}

void MasterBoardComm::update()
{
    MsgPacketizer::update();
}

String MasterBoardComm::work()
{
    auto auto_queue_flush = Common::QueueAutoFlush<typeof(m_queue)>(m_queue);

    for (MasterBoardComm::queue_t::queue_ret ret = MasterBoardComm::m_queue.pop(); ret.ret_code; ret = MasterBoardComm::m_queue.pop())
    {
        if (ret.member == Messages::CmdOps::FORCE)
        {
            return "FORCE";
        }

        if (ret.member == Messages::CmdOps::AUTO)
        {
            return "AUTO";
        }

        if (ret.member == Messages::CmdOps::START)
        {
            return "START";
        }

        if (ret.member == Messages::CmdOps::STOP)
        {
            return "STOP";
        }
    }

    return "";
}