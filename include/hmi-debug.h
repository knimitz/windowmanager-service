#ifndef __HMI_DEBUG_H__
#define __HMI_DEBUG_H__

#include <time.h>
#include <stdio.h>
#include <afb/afb-binding.h>

enum LOG_LEVEL{
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX = LOG_LEVEL_ERROR
};

#define HMI_ERROR(prefix, args,...) _HMI_LOG(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, prefix, args, ##__VA_ARGS__)
#define HMI_WARNING(prefix, args,...) _HMI_LOG(LOG_LEVEL_WARNING, __FILE__, __FUNCTION__,__LINE__, prefix, args,##__VA_ARGS__)
#define HMI_NOTICE(prefix, args,...) _HMI_LOG(LOG_LEVEL_NOTICE, __FILE__, __FUNCTION__,__LINE__, prefix, args,##__VA_ARGS__)
#define HMI_INFO(prefix, args,...)  _HMI_LOG(LOG_LEVEL_INFO, __FILE__, __FUNCTION__,__LINE__, prefix, args,##__VA_ARGS__)
#define HMI_DEBUG(prefix, args,...) _HMI_LOG(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__,__LINE__, prefix, args,##__VA_ARGS__)

static char ERROR_FLAG[6][20] = {"NONE", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

static void _HMI_LOG(enum LOG_LEVEL level, const char* file, const char* func, const int line, const char* prefix, const char* log, ...)
{
    const int log_level = (getenv("USE_HMI_DEBUG") == NULL)?0:atoi(getenv("USE_HMI_DEBUG"));
    if(log_level < level)
    {
        return;
    }

    char *message;
    struct timespec tp;
    unsigned int time;

    clock_gettime(CLOCK_REALTIME, &tp);
	time = (tp.tv_sec * 1000000L) + (tp.tv_nsec / 1000);

	va_list args;
	va_start(args, log);
	if (log == NULL || vasprintf(&message, log, args) < 0)
        message = NULL;
    fprintf(stderr,  "[%10.3f] [%s %s] [%s:%d] >>> %s \n", time / 1000.0, prefix, ERROR_FLAG[level], func, line, message);
    va_end(args);
	free(message);
}

#endif  //__HMI_DEBUG_H__