#ifndef __LOGGING__
#define __LOGGING__
#include <string>
#include <atomic>
namespace logging 
{


namespace level{
    typedef enum{
        trace = 0,
        debug = 1,
        info = 2,
        warn = 3,
        err = 4,
        fatal = 5
    } level_enum;

#define LEVEL_NAMES { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" }
static const char* level_names[] LEVEL_NAMES;

inline const char* to_str(logging::level::level_enum l){
    return level_names[l];
}
}

class Logger{
public:
    Logger();
    ~Logger();
    Logger(const Logger&) =delete;
    Logger& operator=(const Logger&)=delete;
    void setFileName(const std::string& filename);
    void setLogLevel(level::level_enum l);
    level::level_enum getLogLevel(){
        return level_;
    }

    int getFd() {
        return fd_;
    }
    void logv(int level, const char* file, int line, const char* funct, const char * fmt, ...);
    Logger& getLogger();
    
    void maybeRotate();

private:
    int fd_;
    level::level_enum level_;
    std::string filename_;

    std::atomic<int64_t> realRotate_;
    long lastRotate_;
    long rotateInterval_;
};

}

#endif
