#ifndef LOG_H
#define LOG_H

#define LOG_FILE_SIZE_DEFAULT 1000000

///带换行的printf,内容的最大长度是2048
void printfln(const char *fmt,...);
///配置输出调试信息
void cc_log_config_file(const char *filename,long long log_file_size);
///写入log到文件
void cc_log_config_info(int showinfo);
///输出调试日志信息
void _cc_log(const char *fmt,...);
///输出系统错误信息
void cc_log_error();
#define cc_log(fmt,...) _cc_log("[%s:%d:%s]:"fmt,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#endif
