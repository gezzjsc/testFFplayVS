/* cli_log.h */

#ifndef _CLI_LOG_H_H_H_
#define _CLI_LOG_H_H_H_

#include <sys/types.h>

#define MAX_PATHNAME_LENGTH     256
#define MAX_FILENAME_LENGTH     128
#define MAX_LOG_FULL_PATH       (MAX_PATHNAME_LENGTH + MAX_FILENAME_LENGTH)
#define MAX_TIME_STRING_LENGTH  128

typedef struct cli_log_cfg_s {
    char    path[MAX_PATHNAME_LENGTH];
    char    logfile[MAX_FILENAME_LENGTH];
    char    lockfile[MAX_FILENAME_LENGTH];
    size_t  file_size;
    int     roll_num;
} cli_log_cfg_t;

typedef enum log_level_s {
    DEBUG_LEVEL = 1,
    WARNING_LEVEL,
    ERROR_LEVEL,
    NULL_LEVEL
}log_level_t;

/* init_log_cfg 生成日志默认配置文件 */
extern int init_log_cfg();

/**
 * write_log - 写日志信息到日志文件中
 *
 * @level:  日志写入等级，
 *          DEBUG_LEVEL     debug调试信息
 *          WARNING_LEVEL   警告信息
 *          ERROR_LEVEL     错误信息
 *          NULL_LEVEL      不输出包含等级的日志信息
 * @fmt:    日志格式
 *
 * 功能：对日志进行写入操作，当日志文件达到一定的大小后，进行回滚备份操作；
 *
 */
extern int write_log(log_level_t level, char *fmt, ...);

#endif /* _CLI_LOG_H_H_H_ */
