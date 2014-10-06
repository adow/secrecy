/*
 *  log.c 
 *  darkjundle 
 *
 *  Created by adow on 2014-09-09 15:26:33 
 *  Copyright (C) 20014 adow, All rights reserved.
 *  
 *  gcc log.c 
 */
#include <errno.h>		/* for definition of errno */
#include <stdarg.h>		/* ISO C variable aruments */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>

#include "log.h"

#define LOG_MAX_LINE 2048 ///每行日志的最大长度
#define LOG_DEBUG_INFO_BUF_LENGTH 100 ///调试信息的缓冲区长度
#define LOG_PATH_SPLIT "/" ///路径中的目录分隔符

static FILE *log_file_output;///输出log的文件
static char log_file_name[PATH_MAX];///输出日志的文件名
static long long log_file_max_size=LOG_FILE_SIZE_DEFAULT;///文件的最大长度
static long long log_file_current_size=0;///现在的文件的长度

///带换行的printf,内容的最大长度是2048
void printfln(const char *fmt,...){
	va_list ap;
	va_start(ap,fmt);
	char buf[LOG_MAX_LINE]={'\0'};
	vsnprintf(buf,LOG_MAX_LINE-1,fmt,ap);
	va_end(ap);
	printf("%s\n",buf);
}
///当前时间
static char *_debug_time(){
	static char buf[LOG_DEBUG_INFO_BUF_LENGTH]={'\0'};
	memset(buf,'\0',LOG_DEBUG_INFO_BUF_LENGTH);
	time_t t=time(NULL);
	struct tm *tm=localtime(&t);
	strftime(buf,LOG_DEBUG_INFO_BUF_LENGTH-1,"%Y-%m-%d %H:%M:%S",tm);
	return buf;	
}
///取得文件中的目录所在的位置
static char *_dir_filename(const char *filename){
	static char dir[PATH_MAX]={'\0'};
	memset(dir,'\0',PATH_MAX);
	char *p=strstr(filename,LOG_PATH_SPLIT);
	if (p){
		while (p){
			p+=1;
			if (strstr(p,LOG_PATH_SPLIT)){
				p=strstr(p,LOG_PATH_SPLIT);
			}		
			else{
				strncpy(dir,filename,p-filename);
				//printf("filename:%s\n",p);
				break;
			}	
		}
		return dir;
	}
	else{
		return "";
	}
}
///目录下的文件数量
static int _dir_file_total(const char *path){
	DIR *dir=opendir(path);
	if (!dir){
		return -1;
	}
	struct dirent *dirent;
	int total=0;
	while((dirent=readdir(dir))){
		printf("%s\n",dirent->d_name);
		if (!strcmp(dirent->d_name,".") || !strcmp(dirent->d_name,"..")){
            		continue;
		}
		total++;
	}
	closedir(dir);
	return total;
}
///配置日志输出文件,filename 为 空就不输出log,log_file_size 为日志文件大小，0为默认文件大小
void cc_log_config_file(const char *filename,long long log_file_size){
	if (log_file_output){
		fclose(log_file_output);
		log_file_output=NULL;
		memset(log_file_name,'\0',PATH_MAX);
	}
	if (filename){
		if (log_file_size>0)
			log_file_max_size=log_file_size;///设置日志文件大小
		memset(log_file_name,'\0',PATH_MAX);
		strcpy(log_file_name,filename);
		log_file_output=fopen(filename,"a");
		struct stat stat;
		lstat(filename,&stat);
		log_file_current_size=stat.st_size;
		printf("log_file_current:%lld\n",log_file_current_size);	
	}
}
///写入log到文件
static void _cc_log_file(const char *log){
	if (log_file_output){
		fputs(log,log_file_output);
		log_file_current_size+=strlen(log);
		if (log_file_current_size>log_file_max_size){
			printf("new log file\n");
			fflush(log_file_output);
			fclose(log_file_output);///关闭文件
			char *dir=_dir_filename(log_file_name);
			int file_total=_dir_file_total(dir);
			char log_filename_new[PATH_MAX]={'\0'};
			sprintf(log_filename_new,
					"%s.%d",log_file_name,file_total);
			rename(log_file_name,log_filename_new);///移动文件
			log_file_output=fopen(log_file_name,"a");///重新打开文件
			log_file_current_size=0;
		}
	}	
}
///输出调试日志信息
void _cc_log(const char *fmt,...){
	va_list ap;
	va_start(ap,fmt);
	char buf[LOG_MAX_LINE]={'\0'};
	vsnprintf(buf,LOG_MAX_LINE-1,fmt,ap);
	va_end(ap);
	char *time=_debug_time();	
	char log[LOG_MAX_LINE]={'\0'};
	sprintf(log,"%s ",time);
	strcat(log,buf);
	strcat(log,"\n");	
	fputs(log,stdout);
	_cc_log_file(log);	
	fflush(NULL);
}
///输出系统错误信息
void cc_log_error(){
	char buf[LOG_DEBUG_INFO_BUF_LENGTH]={'\0'};
	if (errno){
		snprintf(buf,LOG_DEBUG_INFO_BUF_LENGTH-1,"%d:%s",errno,strerror(errno));	
	}
	else{
		strcat(buf,"ok");
	}
	cc_log("%s",buf);
}
///mark - test
void test_cc_log(){
	cc_log_config_file("./a.txt",LOG_FILE_SIZE_DEFAULT);
	cc_log("cc_log:%d",3);
	cc_log("abc");
	int a=0;
	cc_log("a:%p",&a);
	//cc_log_config_file("./a.1.txt");
	cc_log("a.1.txt");
	//cc_log_config_file(NULL);
	cc_log("cc_log");
}
void test_dir_filename(){
	printf("_dir_filename\n");
	//char filename[]="/Users/reynoldqin/a.txt";
	char filename[]="./a.txt";
	char *dir=_dir_filename(filename);
	printf("dir:%s\n",dir);
	int total=_dir_file_total(dir);
	printf("file total:%d\n",total);
}
void test_printfln(){
	int a=123;
	printfln("test:%d",a);
	printfln("a test:%p",&a);
}
/*
int main(int arg_c,char *arg_v[]){
	test_cc_log();
	//test_dir_filename();
	test_printfln();
	return 0;
}
*/
