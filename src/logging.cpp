#include "logging.hpp"
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
namespace logging 
{
    using namespace level;
    Logger::Logger():level_(level_enum::info), lastRotate_(time(0)), rotateInterval_(3600)
    {
        tzset();
        fd_ = -1;
        realRotate_ = lastRotate_;
    }

   


    Logger::~Logger() {
        if (fd_ != -1) {
            close(fd_);
        }
    }

    Logger& Logger::getLogger(){
        static Logger logger;
        return logger;
    }

    void Logger::setLogLevel(level_enum l){
        level_ = l;
    }

    void Logger::setFileName(const std::string& filename){
        int fd = open(filename.c_str(), O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, DEFFILEMODE);
        if(fd < 0){
            fprintf(stderr, "open log file %s failed. msg: %s ignored\n",
                                    filename.c_str(), strerror(errno));
            return;
        }
        filename_ = filename;
        if(fd_ == -1){
            fd_ = fd;
        }
        else {
            int r = dup2(fd, fd_);
            if(r < 0)
                fprintf(stderr, "dup2 failed.\n");
            close(fd);
        }
    }

    void Logger::maybeRotate(){

        return;
    }
    
    static thread_local uint64_t tid;
    void Logger::logv(int level, const char* file, int line , const char* funct, const char* fmt,...){
        if(tid == 0){
            tid = syscall(SYS_gettid);
        }
        if(level > level_){
            return;
        }
        maybeRotate();
        char buffer[4*1024];
        char* p = buffer;
        char* limit = buffer + sizeof(buffer);
        struct timeval now_tv;
        gettimeofday(&now_tv, NULL);
        const time_t seconds = now_tv.tv_sec;
        struct tm t;
        localtime_r(&seconds, &t);
        p += snprintf(p, limit - p,
                "%04d/%02d/%02d-%02d:%02d:%02d.%06d %lx %s %s:%d ",
                t.tm_year + 1900,
                t.tm_mon + 1,
                t.tm_mday,
                t.tm_hour,
                t.tm_min,
                t.tm_sec,
                static_cast<int>(now_tv.tv_usec),
                (long)tid,
                level::to_str((level_enum)level),
                file,
                line);
        va_list args;
        va_start(args, fmt);
        p += vsnprintf(p, limit-p, fmt, args);
        va_end(args);
        
        p = std::min(p, limit-2);
        while(*--p == '\n'){}
        *++p = '\n';
        *++p = '\0';
        int fd = fd_ == -1 ? STDOUT_FILENO : fd_;
        int err = ::write(fd, buffer, p - buffer);
        if(err != p-buffer){
             fprintf(stderr, "write log file %s failed. written %d errmsg: %s\n",
                     filename_.c_str(), err, strerror(errno));
        }

        if(level >= level_enum::err) {
            syslog(LOG_ERR, "%s", buffer + 27);
        }
        if (level == level_enum::fatal) {
            fprintf(stderr, "%s", buffer);
            assert(0);
        }
    }

}

