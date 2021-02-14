#ifndef INC_LOG_H_
#define INC_LOG_H_
#include <stdint.h>

typedef uint8_t eLogLevel;
#define LL_DEBUG 0
#define LL_INFO 1
#define LL_WARN 2
#define LL_ERROR 3

#define ACTIVE_LOG_LEVEL LL_INFO



void stub();
void _log(eLogLevel level, const char* format, ...);

#if LL_DEBUG >= ACTIVE_LOG_LEVEL
#define log_debug(format, ...) _log ( LL_DEBUG, format, ##__VA_ARGS__ )
#else
#define log_debug(format, ...) stub()
#endif

#if LL_INFO >= ACTIVE_LOG_LEVEL
#define log_info(format, ...) _log(LL_INFO, format, ##__VA_ARGS__)
#else
#define log_info(format, ...) stub()
#endif


#if LL_WARN >= ACTIVE_LOG_LEVEL
#define log_warn(format, ...) _log(LL_WARN, format, ##__VA_ARGS__)
#else
#define log_warn(format, ...) stub()
#endif


#if LL_ERROR >= ACTIVE_LOG_LEVEL
#define log_error(format, ...) _log(LL_ERROR, format, ##__VA_ARGS__)
#else
#define log_error(format, ...) stub()
#endif


#endif /* INC_LOG_H_ */
