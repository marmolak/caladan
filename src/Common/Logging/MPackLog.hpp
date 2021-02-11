#pragma once
#include <Arduino.h>


class MPackLog
{
    public:
        static void mlog(const char *const file, const char *const func, const String log);
        //static void mlog(const char *const file, const char *const func, const char *const log);
};

#define MLOG(log_msg) MPackLog::mlog(__FILE__, __func__, log_msg)
