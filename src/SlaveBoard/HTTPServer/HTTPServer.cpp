#include <Arduino.h>
#include <LittleFS.h>

#include "Common/Messages/Cmds.hpp"
#include "SlaveBoard/HTTPServer/HTTPServer.hpp"

void HTTPServer::init()
{
    auto q_push = [this](const Messages::CmdOps op)
    {
        const bool ok = m_q.push(op);
        if (!ok)
        {
            error("Queue is full!");
            return;
        }
    };

    m_server.on("/", [this]()
    {
       send_index();
    });

    m_server.on("/on", [this, q_push]()
    {
        reply_send_on();
        q_push(Messages::CmdOps::START);
    });

    m_server.on("/off", [this, q_push]()
    {
        reply_send_off();
        q_push(Messages::CmdOps::STOP);
    });

    m_server.on("/auto", [this, q_push]()
    {
        reply_send_auto();
        q_push(Messages::CmdOps::AUTO);
    });

    m_server.on("/force", [this, q_push]()
    {
        reply_send_force();
        q_push(Messages::CmdOps::FORCE);
    });

    m_server.begin();
}

void HTTPServer::error(const char *const str)
{
    m_server.send(404, "text/plain", "404: Not Found");
}

void HTTPServer::handle_requests()
{
    m_server.handleClient();

    for (queue_t::queue_ret ret = m_q.pop(); ret.ret_code; ret = m_q.pop())
    {
        switch (ret.member)
        {
            case Messages::CmdOps::AUTO:
                send_auto();
                break;
            case Messages::CmdOps::INDEX:
                break;
            case Messages::CmdOps::START:
                send_on();
                break;
            case Messages::CmdOps::STOP:
                stop_system();
                break;
            case Messages::CmdOps::FORCE:
                send_force();
                break;
        }
    }
}

void HTTPServer::send_to_master(const Messages::CmdOps cmd_op)
{
    const Messages::MessageCmd m = {
        .cmd_op = static_cast<uint8_t>(cmd_op)
    };
    MsgPacketizer::send(Serial, Messages::MessageCmd::code, m);
}

void HTTPServer::send_on()
{
    send_to_master(Messages::CmdOps::START);
}

void HTTPServer::reply_send_on()
{
    m_server.send(200, "text/plain", "Zapiname!");
}

void HTTPServer::reply_send_force()
{
    m_server.send(200, "text/plain", "Forcujeme!");
}

void HTTPServer::send_force()
{
    send_to_master(Messages::CmdOps::FORCE);
}

void HTTPServer::reply_send_auto()
{
    m_server.send(200, "text/plain", "Forcujeme!");
}

void HTTPServer::send_auto()
{
    send_to_master(Messages::CmdOps::AUTO);
}

void HTTPServer::reply_send_off()
{
    m_server.send(200, "text/plain", "Vypiname!");
}

void HTTPServer::send_off()
{
    stop_system();
}

void HTTPServer::stop_system()
{
    send_to_master(Messages::CmdOps::STOP);;
}

void HTTPServer::send_index()
{
    File file = LittleFS.open("index.html", "r");
    if (!file)
    {
        m_server.send(404, "text/plain", "404: Not Found");
        return;
    }

    const size_t sent = m_server.streamFile(file, "text/html");
    file.close();
}