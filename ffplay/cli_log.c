/*
 * cli_log.c
 *  
 * 本程序用于实现对日志信息的记录，写入到指定的文件  https://github.com/hnwlxywns/log/blob/master/cli_log.c
 *
 */
#include <stdio.h>
#ifdef WIN32  /*我晕，编译的时候，居然不认为是WIN32*/
#include <io.h> 
#include <process.h> 
#else
//#include <unistd.h>
#include "unistd.h"  /*用本地的这个吧*/
#endif

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
/*这个又找不到*/
//#include <sys/file.h>
#include <windows.h> 
#include <conio.h>
//////

#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>

#include "cli_log.h"

#ifdef WIN32
#define LOG_FILE_PATH       "D:\\"
#define LOG_LOCK_FILE_NAME  "D:\\cli_log_lockfile.lock"
#else
#define LOG_FILE_PATH       "."
#define LOG_LOCK_FILE_NAME  ".cli_log_lockfile.lock"
#endif
#define LOG_FILE_NAME       "cli.log"
#define LOG_FILE_MAXSIZE    (1*1024*1024)
#define LOG_FILE_ROLL_NUM   2

static cli_log_cfg_t g_cfg_t;

static int logfile_lock(int fd) {
    if (fd < 0) {
        return -1;
    }

    return flock(fd, LOCK_EX);
}

static int logfile_unlock(int fd) {
    if (fd < 0) {
        return -1;
    }
    return flock(fd, LOCK_UN);
}

static size_t get_file_size(char *path) {
    struct stat st;

    if (path == NULL) {
        return -1;
    }

    stat(path, &st);
    return st.st_size;
}

static int is_file_exist(char *file) {
    if (file == NULL) {
        return -1;
    }

    /* access return 0, means file exist, else file no exist. */
    return (access(file, F_OK) ? (-1) : 0);
}

static int is_dir_exist(char *path) {
    return is_file_exist(path);
}

/* 多级目录递归创建 */
static int mk_dirs(char *path) {
    int ret;
    int len;
    char subpath[MAX_PATHNAME_LENGTH]="";
    char *pos = NULL;
    char pth[LOG_FILE_MAXSIZE];

    if (path == NULL) {
        return -1;
    }
    strcpy(pth, path);
    if ((len = strlen(pth)) > 0 && pth[len-1] == '/') {
        pth[len-1] = '\0';
    }

    while((ret = mkdir(pth, S_IRWXU)) != 0) {
        /* 如果文件存在，或是没有创建的权限，或是父目录为只读，则返回 */
        if (errno == EEXIST) {
            return 0;
        } else if (errno == EACCES || errno == EROFS) {
            return ret;
        }
        
        if ((pos = strrchr(pth, '/')) == NULL) {
            return ret;
        }

        strncat(subpath, pth, pos-pth);
        mk_dirs(subpath);
    }
    return ret;
}

/* init_log_cfg 生成日志默认配置文件 */
int init_log_cfg() {
    strcpy(g_cfg_t.path, LOG_FILE_PATH);
    strcpy(g_cfg_t.logfile, LOG_FILE_NAME);
    strcpy(g_cfg_t.lockfile, LOG_LOCK_FILE_NAME);
    g_cfg_t.file_size = (size_t)LOG_FILE_MAXSIZE;
    g_cfg_t.roll_num = (int)LOG_FILE_ROLL_NUM;

    if (mk_dirs(g_cfg_t.path) != 0) {
        return -1;
    } else {
        return 0;   
    }
}

static int get_currenttime(char *currtime) {
    struct tm *ptm = NULL;
    time_t tme;
    char sztime[MAX_TIME_STRING_LENGTH] = "";

    if (currtime == NULL) {
        return -1;
    }

    tme = time(NULL);
    ptm = localtime(&tme);
    sprintf(sztime, "[%d-%02d-%02d %02d:%02d:%02d] ", (ptm->tm_year + 1900), \
            ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, \
            ptm->tm_sec);
    strcpy(currtime, sztime);

    return 0;
}

static int logfile_roll(char *logfile) {
    int i;
    int ret;
    char old_file[MAX_PATHNAME_LENGTH];
    char new_file[MAX_PATHNAME_LENGTH];

    if (logfile == NULL) {
        return -1;
    }

    if (is_file_exist(logfile) == 0) {
        if (g_cfg_t.roll_num == 0) {
            if (remove(logfile) != 0) {
                return -1;
            } else {
                return 0;
            }
        }

        for (i = g_cfg_t.roll_num; i > 0; i--) {
            if (i == 1) {
                sprintf(new_file, "%s", logfile);
            }
            else {
                sprintf(new_file, "%s.%d", logfile, i-1);
            }
            sprintf(old_file, "%s.%d", logfile, i);

            if (is_file_exist(new_file) == 0) {
                if (rename(new_file, old_file) != 0) {
                    return -1;
                }
            }
        }
    }
    return 0;
}

static void get_level_string(log_level_t level, char *msg) {
    if (msg == NULL) {
        return;   
    }

    switch(level) {
        case DEBUG_LEVEL:
            sprintf(msg, "%s\t:", "Debug");
            break;
        case WARNING_LEVEL:
            sprintf(msg, "%s\t:", "Warning");
            break;
        case ERROR_LEVEL:
            sprintf(msg, "%s\t:", "Error");
            break;
        default:
            sprintf(msg, "%s\t:", "Debug");
            break;
    }
}

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
int write_log(log_level_t level, char *fmt, ...) {
    FILE *fd;
    int lock_fd;
    char logpath[MAX_LOG_FULL_PATH] = "";
    char curtime[MAX_TIME_STRING_LENGTH] = "";
    char level_msg[32] = "";
    va_list args;

    if (fmt == NULL) {
        return -1;
    }

    lock_fd = open(g_cfg_t.lockfile, O_WRONLY | O_CREAT, 777);
    if (lock_fd < 0) {
        goto error;
    }

    sprintf(logpath, "%s/%s", g_cfg_t.path, g_cfg_t.logfile);
    /* lock log file */
    logfile_lock(lock_fd);
    fd = fopen(logpath, "a");
    if (fd == NULL) {
        goto error;
    }

    /* Get log file size */
    if (get_file_size(logpath) > g_cfg_t.file_size) {
        /*Roll the log file when log file size greate than config size*/
        if (logfile_roll(logpath) != 0) {
            goto error;
        }
    }
    
    get_currenttime(curtime);
    /* write the message to log file */
    if (level != NULL_LEVEL) {
        get_level_string(level, level_msg);
        fprintf(fd, "%s%s", curtime, level_msg);
    } else {
        fprintf(fd, "%s\t:", curtime);
    }

    /* read va_list */
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);

    fclose(fd);
    /* unlock log file*/
    logfile_unlock(lock_fd);

    close(lock_fd);
    return 0;

error:
    if (fd != NULL) {
        fclose(fd);
    }
    /* unlock log file*/
    logfile_unlock(lock_fd);
    if (lock_fd >= 0) {
        close(lock_fd);
    }
    return -1;
}

