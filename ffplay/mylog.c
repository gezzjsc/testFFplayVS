/*
https://github.com/Leifusheng/mylog

*/
#define  _CRT_SECURE_NO_WARNINGS 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mylog.h"


int  LogLevel[5] = {MYLOG_LEVEL_NOLOG, MYLOG_LEVEL_DEBUG, MYLOG_LEVEL_INFO, MYLOG_LEVEL_WARNING, MYLOG_LEVEL_ERROR };

//Level的名称
char LogLevelName[5][10] = {"NOLOG", "DEBUG", "INFO", "WARNING", "ERROR"};

static int MYLOG_Error_GetCurTime(char* strTime)
{
	struct tm*		tmTime = NULL;
	size_t			timeLen = 0;
	time_t			tTime = 0;
	tTime = time(NULL);
	tmTime = localtime(&tTime);
	timeLen = strftime(strTime, 33, "%Y.%m.%d %H:%M:%S", tmTime);
	return timeLen;
}

static int MYLOG_Error_OpenFile(int* pf)
{
	char	fileName[1024];
	
	memset(fileName, 0, sizeof(fileName));
#ifdef WIN32
	sprintf(fileName, "c:\\%s", MYLOG_DEBUG_FILE);
#else
	sprintf(fileName, "%s/log/", getenv("HOME"));
	if (access(fileName, F_OK) != 0)
	{
		//printf("create dir log\n");
		if (mkdir(fileName, 0777))
		{
			printf("create dir failed\n");
		}
	}
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "%s/log/%s", getenv("HOME"), ITCAST_DEBUG_FILE_);
#endif
    
    *pf = open(fileName, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if(*pf < 0)
    {
        return -1;
    }
	return 0;
}

static void MYLOG_Error_Core(const char *file, int line, int level, int status, const char *fmt, va_list args)
{
    char str[MYLOG_MAX_STRING_LEN];
    int	 strLen = 0;
    char tmpStr[64];
    int	 tmpStrLen = 0;
    int  pf = 0;
    
    //初始化
    memset(str, 0, MYLOG_MAX_STRING_LEN);
    memset(tmpStr, 0, 64);
    
    //加入LOG时间
    tmpStrLen = MYLOG_Error_GetCurTime(tmpStr);
    tmpStrLen = sprintf(str, "[%s] ", tmpStr);
    strLen = tmpStrLen;

    //加入LOG等级
    tmpStrLen = sprintf(str+strLen, "[%s] ", LogLevelName[level]);
    strLen += tmpStrLen;
    
    //加入LOG状态
    if (status != 0) 
    {
        tmpStrLen = sprintf(str+strLen, "[ERRNO is %d] ", status);
    }
    else
    {
    	tmpStrLen = sprintf(str+strLen, "[SUCCESS] ");
    }
    strLen += tmpStrLen;

    //加入LOG信息
    tmpStrLen = vsprintf(str+strLen, fmt, args);
    strLen += tmpStrLen;

    //加入LOG发生文件
    tmpStrLen = sprintf(str+strLen, " [%s]", file);
    strLen += tmpStrLen;

    //加入LOG发生行数
    tmpStrLen = sprintf(str+strLen, " [%d]\n", line);
    strLen += tmpStrLen;
    
    //打开LOG文件
    if(MYLOG_Error_OpenFile(&pf))
	{
		return ;
	}
	
    //写入LOG文件
    write(pf, str, strLen);
    //关闭文件
    close(pf);
    return ;
}


void MYLOG(const char *file, int line, int level, int status, const char *fmt, ...)
{
    va_list args;
	
	//判断是否需要写LOG
	if(level == MYLOG_LEVEL_NOLOG)
	{
		return ;
	}
	
	//调用核心的写LOG函数
    va_start(args, fmt);
    MYLOG_Error_Core(file, line, level, status, fmt, args);
    va_end(args);
    return ;
}

