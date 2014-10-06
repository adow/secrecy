/*
 *  main.c 
 *  concertino 
 *
 *  Created by adow on 2014-08-05 20:03:20 
 *  Copyright (C) 20014 adow, All rights reserved.
 *
 *  gcc lib/polarssl/base64.c lib/polarssl/aes.c lib/polarssl/sha1.c secrecy.c log.c main.c -o secrecy
 *
 *  加密文件
 *  secrecy encrypt -i README.md -o README.secrecy.md -k key -h <hints>
 *  解密文件 
 *  secrecy decrypt -i README.secrecy.md -o README.decrypt.md -k key
 *
 *  -i 输入文件
 *  -o 输出文件，如果不指定 -o,那就覆盖输入的文件
 *  -k 密钥
 *  -h 密钥提示
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "log.h"
#include "secrecy.h"

///输出帮助信息
void _output_help(){
	const char *help="SECRECY COMMANDS:\n"
		"* secrecy encrypt -i <inputfile> -o <outputfile> -k <secret> -h <hints>\n"
		"\t Encrypt a File,-h could be ignored, -k should be less than 256 in length;\n"
		"* secrecy decrypt -i <inputfile> -o <outputfile> -k <secret>\n"
		"\t Decrypt a file, -k should be less than 256 in length;\n"
		"* secrecy test\n"
		"\t Test encrypt and decrypt lipsum.md;\n"
		"* secrecy help\n"
		"\t Help;";
	printfln("%s",help);
}
///处理参数，执行加密或者解密
int _execute_cmd(int arg_c,char *arg_v[]){
	if(arg_c<=1){
		printfln("need a command");
		_output_help();
		return -1;	
	}
	char *cmd=arg_v[1];
	cc_log("cmd:%s",cmd);
	///第一个参数是encrypt/decrpt,所以处理的参数列表要往后面移
	arg_v+=1;
	arg_c-=1;
	char input_filename[PATH_MAX]={'\0'};
	char output_filename[PATH_MAX]={'\0'};
	char hints[PATH_MAX]={'\0'};
	unsigned char key[256]={'\0'};
	char format[]="k:i:o::h::";
	int ch;
	while((ch=getopt(arg_c,arg_v,format))!=-1){
		//printfln("%c",ch);
		switch(ch){
			case 'i':
				strcpy(input_filename,optarg);
				break;
			case 'o':
				strcpy(output_filename,optarg);
				break;
			case 'k':
				strcpy((char *)key,optarg);
				break;
			case 'h':
				strcpy(hints,optarg);
				break;
		}
	}
	if (!strcmp(cmd,"help")){
		_output_help();
		return 0;
	}
	else if (!strcmp(cmd,"test")){
		secrecy_self_test();
		return 0;
	}
	else {
		if (!strlen((char *)key)){
			printfln("key required");
			_output_help();
			return -1;
		}
		if (!strlen(input_filename)){
			printfln("no input filename");
			_output_help();
			return -1;
		}
		if (!strlen(output_filename)){
			strcpy(output_filename,input_filename);	
		}
		cc_log("key:%s",key);
		cc_log("input_filename:%s",input_filename);
		cc_log("output_filename:%s",output_filename);
		cc_log("hints:%s",hints);
		printfln("");
		if (!strcmp(cmd,"encrypt")){
			if (secrecy_encrypt_file(input_filename,output_filename,key,hints)){
				printfln("encrypt error");
				return -1;
			}
			return 0;
		}
		else if (!strcmp(cmd,"decrypt")){
			if (secrecy_decrypt_file(input_filename,output_filename,key,1)){
				printfln("decrypt error");
				return -1;
			}
			return 0;	
		}
		else{
			printfln("command unknown");
			_output_help();
			return -1;
		}

	}	
	return 0;
}

int main(int arg_c,char *arg_v[]){
	//secrecy_self_test();
	_execute_cmd(arg_c,arg_v);
	return 0;
}
