/* Borrowed from yedf/handy
 *
 * 
 *
 *
 */
#ifndef __LOGGING__
#define __LOGGING__
#include <string>
#include <atomic>
#define hlog(level, ...) \
    do { \
        logging::Logger::getLogger().logv(level, __FILE__, __LINE__, __func__, __VA_ARGS__);    \
    } while(0)



#define TRACE(...) hlog(logging::level::level_enum::trace, __VA_ARGS__)
#define DEBUG(...) hlog(logging::level::level_enum::debug, __VA_ARGS__)
#define INFO(...) hlog(logging::level::level_enum::info, __VA_ARGS__)
#define WARN(...) hlog(logging::level::level_enum::warn, __VA_ARGS__)
#define ERROR(...) hlog(logging::level::level_enum::error, __VA_ARGS__)
#define FATAL(...) hlog(logging::level::level_enum::fatal, __VA_ARGS__)
#define FATALIF(b, ...) do { if((b)) { hlog(logging::level::level_enum::fatal, __VA_ARGS__); } } while (0)
#define CHECK(b, ...) do { if((b)) { hlog(logging::level::level_enum::fatal, __VA_ARGS__); } } while (0)
#define EXITIF(b, ...) do { if ((b)) { hlog(logging::level::level_enum::error, __VA_ARGS__); _exit(1); }} while(0)
namespace logging 
{


namespace level{
    typedef enum{
        trace = 0,
        debug = 1,
        info = 2,
        warn = 3,
        error = 4,
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
    void setRotateInterval_(long rotateInterval){
        rotateInterval_ = rotateInterval;
    }
        
    level::level_enum getLogLevel(){
        return level_;
    }

    int getFd() {
        return fd_;
    }
    void logv(int level, const char* file, int line, const char* funct, const char * fmt, ...);
    static Logger& getLogger();
    
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
