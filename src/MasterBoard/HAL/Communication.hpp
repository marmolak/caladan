#pragma once

#include <Arduino.h>

#include "Common/Queue/Queue.hpp"
#include "Common/Messages/Cmds.hpp"

class MasterBoardComm
{
    public:
        void init();
        void update();
        String work();

        MasterBoardComm() = default;

        // Don't copy/move that floppy.
        MasterBoardComm(const MasterBoardComm &) = delete;
        MasterBoardComm(MasterBoardComm &)       = delete;
        MasterBoardComm(MasterBoardComm &&)      = delete;

        using queue_t = Common::Queue<Messages::CmdOps, 10>;
    private:
        // declare as static because MsgPacker have some issues with capturing this pointer,
        // which leads to funny bugs... like empty queue.
        static queue_t m_queue; 
};