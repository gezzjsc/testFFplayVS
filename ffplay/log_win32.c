#include "log_win32.h"
#include <stdarg.h>
/*
http://stackoverflow.com/questions/14386/fopen-deprecated-warning
'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
*/
#pragma warning (disable : 4996)

FILE * pLog = NULL;
bCreateFile=0;
#define _DEBUG_ 

/*这个可以用*/
#include <time.h>	//利用时间生成种子
#include <sys/timeb.h>

//获取系统时间,精确到毫秒 https://github.com/KingCoolS520/MyRepos/blob/386c43befc97d3b61a643cb55cbb1707f0ebf22b/2.%E6%8E%92%E5%BA%8F__all/%E6%8E%92%E5%BA%8F/demo08_%E4%B8%83%E7%A7%8D%E6%8E%92%E5%BA%8F%E6%95%88%E7%8E%87%E6%AF%94%E8%BE%83%E6%B5%8B%E8%AF%95.c
long long getSystemTime()
{
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}


/* 这个可以到毫秒
To get the time expressed as UTC, use GetSystemTime in the Win32 API.

SYSTEMTIME st;
GetSystemTime(&st);

SYSTEMTIME is documented as having these relevant members:

WORD wYear;
WORD wMonth;
WORD wDayOfWeek;
WORD wDay;
WORD wHour;
WORD wMinute;
WORD wSecond;
WORD wMilliseconds;

http://stackoverflow.com/questions/1695288/getting-the-current-time-in-milliseconds-from-the-system-clock-in-windows
sprintf(currentTime,"%.4d%.2d%.2d%.2d%.2d%.2d%.4d",st.wYear,‌​st.wMonth,st.wDay, st.wHour, st.wMinute, st.wSecond , st.wMilliseconds);
*/
#include "windows.h"
static int getCurTime(char* strTime)
{
	SYSTEMTIME st, lt;
	GetSystemTime(&st);
	/*[201705030830090690] "%.4d%.2d%.2d%.2d%.2d%.2d%.4d" */
	int strlen=sprintf(strTime, "%.4d%.2d%.2d %.2d_%.2d_%.2d_%.4d", st.wYear,st.wMonth,st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return strlen;
}
#ifdef _DEBUG_
#if DEBUG_OUTPUT_MODE == DEBUG_OUTPUT_IDE
#include <windows.h>


void log_to_ide(const char * format, ...)
{
	char szBuff[1024];
	va_list arg;
	va_start(arg, format);
	_vsnprintf_s(szBuff, sizeof(szBuff), format, arg);
	va_end(arg);
	OutputDebugStringA(szBuff);
	OutputDebugStringA("\n");
}


#elif DEBUG_OUTPUT_MODE == DEBUG_OUTPUT_FILE
#define MYLOG_MAX_STRING_LEN 		10240

void log_to_file(const char * format,...)
{
    if(bCreateFile == 0){
       pLog= fopen(DEBUG_FILENAME, "a");
	   fprintf(stderr,"DEBUG FILE [%s]",DEBUG_FILENAME);
       if(pLog!=NULL){
            bCreateFile = 1;
       }
    }    
	if (pLog == NULL){
		return;
    }
	int	 strLen = 0;
	char tmpStr[64];
	int	 tmpStrLen = 0;
#if 0
	char str[MYLOG_MAX_STRING_LEN];


	//初始化
	memset(str, 0, MYLOG_MAX_STRING_LEN);
	memset(tmpStr, 0, 64);

	//加入LOG时间
	tmpStrLen = GetCurTime(tmpStr);
	strLen += tmpStrLen;

	//加入传入信息
	tmpStrLen = vsprintf(str + strLen, format, args);
	strLen += tmpStrLen;
#else

	memset(tmpStr, 0, 64);
	//加入LOG时间
	tmpStrLen = getCurTime(tmpStr);
	strLen += tmpStrLen;
	fprintf(pLog,"[%s]", tmpStr);

#endif
	va_list args;
    va_start(args, format);
    vfprintf(pLog, format, args);
	fputc('\n', pLog);
    va_end(args);
	fclose(pLog);

}
void log_to_file_original(const char * format ,...) {
	if (bCreateFile == 0) {
		pLog = fopen(DEBUG_FILENAME, "a");
		fprintf(stderr, "DEBUG FILE [%s]", DEBUG_FILENAME);
		if (pLog != NULL) {
			bCreateFile = 1;
		}
	}
	if (pLog == NULL) {
		return;
	}
	va_list args;
	va_start(args, format);
	vfprintf(pLog, format, args);
	fputc('\n', pLog);
	va_end(args);
	fclose(pLog);
}

int myvprintf(const char *fmt, va_list args) {
			return vfprintf(stdout, fmt, args);
	
}

#if 0 
//https://github.com/protomuck/proto1/blob/2f78525eae65e935a7245dd87dbfccdbfa47f260/protomuck/src/log.c
void
log2filetime(char *myfilename, char *format, ...)
{
	char wall[BUFFER_LEN];
	va_list args;
	FILE *fp;
	time_t lt;
	char buf[40];

	lt = current_systime;
	va_start(args, format);

	*buf = '\0';
	if ((fp = fopen(myfilename, "a")) == NULL) {
		fprintf(stderr, "Unable to open %s!\n", myfilename);
		fprintf(stderr, "%.16s: ", ctime(&lt));
		vsprintf(wall, format, args);
		fprintf(stderr, "%s", wall);
	}
	else {
		format_time(buf, 32, "%c\0", localtime(&lt));
		fprintf(fp, "%.32s: ", buf);
		vsprintf(wall, format, args);
		fprintf(fp, "%s", wall);
		fclose(fp);
	}
	va_end(args);
}
#endif

#endif
#endif

