#pragma once

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <mutex>

// Console + file logger.
//   client: <game dir>\coopandreas.log         (cwd of gta_sa.exe)
//   server: <server dir>\coopandreas-server.log (main() sets cwd to the exe dir)
// The file is appended to and flushed on every line, so the last lines survive a crash.
class logger
{
private:
    static const char* FileName()
    {
#ifdef COOP_SERVER
        return "coopandreas-server.log";
#else
        return "coopandreas.log";
#endif
    }

    static FILE*& File()
    {
        static FILE* file = nullptr;
        return file;
    }

    static std::mutex& Mutex()
    {
        static std::mutex m;
        return m;
    }

    static void WriteFile(const char* text, const char* level)
    {
        std::lock_guard<std::mutex> lock(Mutex());

        FILE*& file = File();
        if (file == nullptr)
        {
            file = std::fopen(FileName(), "a");
            if (file == nullptr)
                return;
            std::fprintf(file, "\n========== log opened ==========\n");
        }

        std::time_t now = std::time(nullptr);
        std::tm tmNow{};
#ifdef _WIN32
        localtime_s(&tmNow, &now);
#else
        localtime_r(&now, &tmNow);
#endif
        char stamp[32];
        std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tmNow);

        std::fprintf(file, "[%s] [%s] %s\n", stamp, level, text);
        std::fflush(file);
    }

    static void log(const char* text, const char* level, const char* color)
    {
        // file first: if the console blocks (e.g. Windows QuickEdit "Select" mode),
        // the event is still on disk
        WriteFile(text, level);
        std::cout << color << "[" << level << "]" << "\033[0m" << ": " << text << std::endl;
    }

public:
    static void info(const char* format, ...)
    {
        char buffer[512];
        va_list args;
        va_start(args, format);
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        log(buffer, "info", "\033[34m");
    }

    static void warn(const char* format, ...)
    {
        char buffer[512];
        va_list args;
        va_start(args, format);
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        log(buffer, "warn", "\033[33m");
    }

    static void error(const char* format, ...)
    {
        char buffer[512];
        va_list args;
        va_start(args, format);
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        log(buffer, "error", "\033[31m");
    }
};
