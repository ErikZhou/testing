#include <stdio.h>
 
#define LOG_TURN_ON
	
#ifdef LOG_TURN_ON
#define log_info(...) do {\
LOG_isEnabled()&&\
printf("<%s %s> %s %s:%d INFO:",__DATE__, __TIME__,__FUNCTION__,__FILE__,__LINE__)&&\
printf(__VA_ARGS__)&&\
printf("\n");\
}while(0)
//__DATE__, __TIME__,__FUNCTION__,__FILE__,__LINE__ 日期宏，时间宏，函数宏，文件宏，行宏
//__VA_ARGS__ 动态参数宏
#define log_warn(...) do {\
LOG_isEnabled()&&\
printf("<%s %s> %s %s:%d WARN:",__DATE__, __TIME__,__FUNCTION__,__FILE__,__LINE__)&&\
printf(__VA_ARGS__)&&\
printf("\n");\
}while(0)
 
#define log_error(...) do {\
LOG_isEnabled()&&\
printf("<%s %s> %s %s:%d ERROR:",__DATE__, __TIME__,__FUNCTION__,__FILE__,__LINE__)&&\
printf(__VA_ARGS__)&&\
printf("\n");\
}while(0)
 
#define log_debug(...) do {\
LOG_isEnabled()&&\
printf("<%s %s> %s %s:%d DEBUG:",__DATE__, __TIME__,__FUNCTION__,__FILE__,__LINE__)&&\
printf(__VA_ARGS__)&&\
printf("\n");\
}while(0)
 
#define log_test(format, ...) do {\
printf(format, __VA_ARGS__);\
printf("\n");\
}while(0)
 
#else
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_debug(...)
#define log_test(format, ...) 
#endif 
 
int logSwitch = 0;	//初始化为关闭状态
bool LOG_isEnabled()
{
  return logSwitch;
}	
		
int main()
{  
  logSwitch = 1; //打开日志开关
  int a = 10;
  int b = 20;
  char c = 'c';
  char d = 'd';
  
  log_info("a =%d b=%d c=%c d=%c", a,b ,c,d);
  log_warn("I'm log_warn ");
  log_error("I'm log_error ");
  log_debug("I'm log_debug ");
  return 1;
}
