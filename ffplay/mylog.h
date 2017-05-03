//written by Frank_Lei
//20150423  https://github.com/Leifusheng/mylog
//log.h 日志头文件

#ifndef _MY_LOG_H_
#define _MY_LOG_H_

//定义log等级
#define MYLOG_LEVEL_NOLOG			0
#define MYLOG_LEVEL_DEBUG			1
#define MYLOG_LEVEL_INFO			2
#define MYLOG_LEVEL_WARNING			3
#define MYLOG_LEVEL_ERROR			4

//文件名称和大小
#define MYLOG_DEBUG_FILE			"mylog.log"
#define MYLOG_MAX_STRING_LEN 		10240

/************************************************************************ 
const char *file：文件名称
int line：文件行号
int level：错误级别
		0 -- 没有日志
		1 -- debug级别
		2 -- info级别
		3 -- warning级别
		4 -- err级别
int status：错误码
const char *fmt：可变参数
************************************************************************/
extern int  LogLevel[5];
void MYLOG(const char *file, int line, int level, int status, const char *fmt, ...);

#endif

