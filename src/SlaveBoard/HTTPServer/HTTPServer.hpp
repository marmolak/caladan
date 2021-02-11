#pragma once

#include <ESP8266WebServer.h>

#include "Common/Messages/Cmds.hpp"
#include "Common/Queue/Queue.hpp"

class HTTPServer
{
    public:
        void init();
        void handle_requests();

    private:
        using queue_t = Common::Queue<Messages::CmdOps, 10>;
        queue_t m_q;

        ESP8266WebServer m_server { 80 };

        void error(const char *const str);

        void send_to_master(const Messages::CmdOps cmd_op);

        void send_index();
        
        void reply_send_on();
        void send_on();

        void reply_send_force();
        void send_force();

        void reply_send_auto();
        void send_auto();

        void reply_send_off();
        void send_off();

        void stop_system();
};