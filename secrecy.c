/*
 *  secrecy.c 
 *  concertino 
 *
 *  Created by adow on 2014-08-05 20:03:20 
 *  Copyright (C) 20014 adow, All rights reserved.
 *
 *  gcc lib/polarssl/base64.c lib/polarssl/aes.c lib/polarssl/sha1.c log.c secrecy.c 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <regex.h>

#include "lib/polarssl/base64.h"
#include "lib/polarssl/config.h"
#include "lib/polarssl/aes.h"
#include "lib/polarssl/sha1.h"
#include "log.h"

#define SECRECY_AES_KEY_LENGTH 256 ///密钥长度
#define SECRECY_BUFFER_LONG_LENGTH 1024000 ///临时缓冲区的最大长度
#define SECRECY_AES_BLOCK_LENGTH 16 ///aes块长度
#define SECRECY_SHA1_LENGTH 20 ///sha1结果长度
#define SECRECY_SHA1_BASE64_LENGTH 100 ///sha1 结果转换为base64时的最大长度
#define SECRECY_TEST_SECRET_KEY "secrecy test key" ///测试用的密钥
#define SECRECY_TEST_FILE "lipsum.md" ///测试用的加密解密文件

///把不可见的字符转换成 int 打印出来
static void print_char_code(unsigned char *str,size_t length){
	for(size_t a=0;a<length;a++){
		printf("%d ",*(str+a));	
	}
	printf("\n");
}
///把char * 中的字符用 int 的的方式输出来,输入的字符串里面可能包含了\0 或者其他不可见的字符,每个字符间用空格分开
static char *string_to_codes(unsigned char *input,
		size_t input_length,
		char *output){
	for(size_t a=0;a<input_length;a++){
		char block[5]={'\0'};
		sprintf(block,"%d ",*input++);
		strcat(output,block);
	}
	return output;	
}
///把用数字显示的字符转换成字符串，这个字符串里面可能包含\0 或者其他不可见的字符
static size_t codes_to_string(char *printable_string,
		char *output){
	size_t length=0;
	char character[3]={'\0'};///每个字符最多有三位数组成
	int character_index=0;
	char *p_output=output;
	char c;
	while((c=*printable_string++)){
		if (c==' '){///获得空格时，把前面去的的三位数字对应的一个字符取出来，作为一个输出的字符
			char output_char=atoi(character);
			*p_output++=output_char;
			character_index=0;///准备开始获取下一个字符
			memset(character,'\0',3);
			continue;	
		}
		else{
			character[character_index++]=c;	
		}	
	}
	if (character_index){///剩下最后的几个数字，作为作为一个字符
		char last_char=atoi(character);
		*p_output++=last_char;
	}
	return p_output-output;	
}
//创建一个输出块
//-----block_name-----
//block_content\n
static char *_secrecy_make_block(const char *block_name,
		const char *block_content,
		char *output){
	sprintf(output,
			"---------%s---------\n"
			"%s\n",
			block_name,block_content);
	return output;
}
///取得一个输出块内的内容
static char *_secrecy_get_block_content(const char *block,
		const char *block_name,
		char *block_content){
	char block_name_line[100]={'\0'};
	sprintf(block_name_line,
			"---------%s---------\n",
			block_name);
	char *p_block=strstr(block,block_name_line);
	if (!p_block)
		return NULL;
	p_block+=strlen(block_name_line);///移动到内容开始
	for(char *p_block_content=block_content;
			*p_block!='\n' && *p_block;
			p_block++){
		*p_block_content++=*p_block;	
	}
	return block_content;
}
///读取文件filename内容
char *file_read(const char *filename){
	static char buffer[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	memset(buffer,'\0',SECRECY_BUFFER_LONG_LENGTH);
	FILE *file=fopen(filename,"r");
	if (!file){
		printfln("file not found:%s",filename);
		return NULL;
	}
	char *p=buffer;
	char c;
	while((c=fgetc(file))!=EOF){
		*p++=c;
	}
	fclose(file);
	return buffer;
}
///写入字符串到filename
int file_write(const char *filename,
		const char *str){
	FILE *file=fopen(filename,"w");
	if (!file){
		printfln("write file error:%s",filename);
		return -1;
	}
	char c;
	while((c=*str++)){
		fputc(c,file);
	}
	fclose(file);
	return 0;
}
///把输入字符串做sha1并输出为base64格式
//input: 需要sha1的字符串
//返回 base64字符串
static unsigned char *sha1_str_to_base64(const char *input){
	unsigned char hash[SECRECY_SHA1_LENGTH]={'\0'};
	sha1((unsigned char *)input,strlen(input),hash);
	static unsigned char base64_hash[SECRECY_SHA1_BASE64_LENGTH]={'\0'};
	memset(base64_hash,'\0',SECRECY_SHA1_LENGTH);
	size_t base64_hash_length=SECRECY_SHA1_BASE64_LENGTH;	
	base64_encode(base64_hash,
			&base64_hash_length,
			hash,
			SECRECY_SHA1_LENGTH);
	return base64_hash;
}
// 使用 aes_cbc_256进行加密
// mode AES_ENCRYPT,AES_DECRYPT 
// key 密钥
// input 输出内容
// output 输出内容 
// 返回 output，如果出错，返回NULL
size_t secrecy_aes(int mode,
		const unsigned char *key,
		const unsigned char *input,
		size_t input_length,
		unsigned char *output){
	//printfln("> secrecy_aes_256");
	//printfln("input:%s",input);
	//printfln("input_length:%ld",input_length);

	aes_context aes_enc;
	unsigned char iv[SECRECY_AES_BLOCK_LENGTH]={'\0'};

	///填充key到256位
	unsigned char key_full[SECRECY_AES_KEY_LENGTH+1]={'\0'};
	memset((char *)key_full,'*',SECRECY_AES_KEY_LENGTH);
	strncpy((char *)key_full,(char *)key,strlen((char *)key));
	//printfln("key_full:%ld:%s",strlen((char *)key_full),key_full);

	///设置加密还是解密
	if (mode==AES_ENCRYPT){
		if(aes_setkey_enc(&aes_enc, key_full , SECRECY_AES_KEY_LENGTH)){
			printfln("aes_setkey_enc error");
			return -1;
		}
	}
	else if (mode==AES_DECRYPT){
		if (aes_setkey_dec(&aes_enc,key_full,SECRECY_AES_KEY_LENGTH)){
			printfln("aes_setkey_enc error");
			return -1;
		}	
	}
	else{
		printfln("unknown mode");
		return -1;
	}

	unsigned char input_block[SECRECY_AES_BLOCK_LENGTH+1]={'\0'}; ///每一块内容长16
	size_t a=0;
	unsigned char *p_output=output;
	const unsigned char *p_input=input;
	///循环每个字节，按照16的长度分组,每16个字符串进行一次加密，加密后拼接到output上去
	while(p_input-input<input_length){
		input_block[a++]=*p_input++;
		if (a>=SECRECY_AES_BLOCK_LENGTH){
			//printfln("a:%d",a);
			/*
			printfln("input_block:%ld:%s",
					strlen((char *)input_block),
					input_block);
			*/
			unsigned char crypt_block[100]={'\0'};
			if (aes_crypt_cbc(&aes_enc,mode,
						SECRECY_AES_BLOCK_LENGTH,
						iv,
						input_block,
						crypt_block)){
				printfln("aes error");
				break;
			}
			///print crypt_block
			//printfln("crypt_block:");
			//print_char_code(crypt_block,SECRECY_AES_BLOCK_LENGTH);
			///	
			memcpy(p_output,crypt_block,SECRECY_AES_BLOCK_LENGTH);///复制到现在的位置
			p_output+=SECRECY_AES_BLOCK_LENGTH;///复制完成之后要移动指针到结尾
			///每16个长度之后，重新填充输入块，用\0填充多余的字节
			a=0;
			memset(input_block,'\0',SECRECY_AES_BLOCK_LENGTH);
		}
	}
	///最后可能留下一个未满16字节的块，单独填充，用\0填充多余的字节
	if (strlen((char *)input_block)){
		//printfln("last_input_block:%ld:%s",a,input_block);
		unsigned char crypt_block[100]={'\0'};
		if (aes_crypt_cbc(&aes_enc,mode,
					SECRECY_AES_BLOCK_LENGTH,
					iv,
					input_block,
					crypt_block)){
			printfln("aes error");
			
		}
		///
		//printfln("last_crypt_block:");
		//print_char_code(crypt_block,SECRECY_AES_BLOCK_LENGTH);
		///
		memcpy(p_output,crypt_block,SECRECY_AES_BLOCK_LENGTH);
		p_output+=SECRECY_AES_BLOCK_LENGTH;///移动到结尾
	}
	size_t output_length=p_output-output; ///输出内容的长度
	//printfln("output_length:%ld",output_length);
	//printfln("output:%s",output);
	//print_char_code(output,output_length);
	return output_length;
}
///加密一个文件
// input_filename: 要加密的文件
// output_filename: 加密后保存的文件
// key: 密钥
// hints: 提示
// 返回 0 成功，其他出错
int secrecy_encrypt_file(const char *input_filename,
		const char *output_filename,
		const unsigned char *key,
		const char *hints
		){
	char output[SECRECY_BUFFER_LONG_LENGTH]={'\0'};

	char *input=file_read(input_filename);

	///encrypt block	
	unsigned char encrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	size_t encrypt_length=secrecy_aes(AES_ENCRYPT,
			key,
			(unsigned char *)input,
			strlen((char *)input),
			encrypt);

	char printable_string[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	string_to_codes(encrypt,
			encrypt_length,
			printable_string);

	unsigned char output_block_encrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	_secrecy_make_block("AES-CBC-256",
			printable_string,
			(char *)output_block_encrypt);
	strcat(output,(char *)output_block_encrypt);

	///hash block
	unsigned char *base64_hash=sha1_str_to_base64(input);
	unsigned char output_block_hash[SECRECY_SHA1_BASE64_LENGTH]={'\0'};
	_secrecy_make_block("SHA",
			(char *)base64_hash,
			(char *)output_block_hash);
	strcat(output,(char *)output_block_hash);

	///hints block
	if (hints && strlen(hints) > 0){
		char output_block_hints[SECRECY_SHA1_BASE64_LENGTH]={'\0'};
		_secrecy_make_block("HINTS",
				hints,output_block_hints);
		strcat(output,output_block_hints);
	}
	///write file
	file_write(output_filename,output);

	printfln("%s\n",output);
	return 0;
}
/// 解密一个文件
// input_filename: 需要解密的文件
// output_filename: 解密后保存的文件
// key: 密钥
// check_sha: 是否需要校验 sha
// 返回0 成功
int secrecy_decrypt_file(const char *input_filename,
		const char *output_filename,
		const unsigned char *key,
		int check_sha
		){
	char output[SECRECY_BUFFER_LONG_LENGTH]={'\0'};

	char *input=file_read(input_filename);
	printfln("input:\n%s",input);

	///encrypt
	char codes_encrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	_secrecy_get_block_content(input,"AES-CBC-256",codes_encrypt);
	char encrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	size_t encrypt_length=codes_to_string(codes_encrypt,encrypt);
	//printfln("encrypt:%ld,%s",encrypt_length,encrypt);

	///aes	
	size_t output_length=secrecy_aes(AES_DECRYPT,
			key,
			(unsigned char *)encrypt,
			encrypt_length,
			(unsigned char *)output);

	if (check_sha){
		///sha1
		char sha1_from_input[SECRECY_SHA1_BASE64_LENGTH]={'\0'};
		_secrecy_get_block_content(input,"SHA",sha1_from_input);	
		unsigned char *sha1_check=sha1_str_to_base64(output);
		if (strcmp(sha1_from_input,(char *)sha1_check)==0){
			printfln("check sha1 succeed");
		}
		else{
			printfln("check sha failed:%s,%s",sha1_from_input,
					sha1_check);
			return -1;
		}
	}

	file_write(output_filename,output);
	printfln("output:\n%s",output);
	return 0;
}
///test
void test_secrecy_aes(){
	//unsigned char key[]="Answer2LifeUniverseAndEverything";
	unsigned char key[]="key";
	//unsigned char input[]="abcdefghijk\n \nlmnopqrstuvwxyz\n0123456789";
	//unsigned char *input=(unsigned char *)file_read("./cc.c");
	//unsigned char input[]="# Concertina\n \n*";
	//unsigned char input[]="5624421742034243274200250898126767144663061557465289675958910652abcdefghijklmnopqrstuvwxyz1235624421742034243274200250898126767144663061557465289675958910652abcdefghijklmnopqrstuvwxyz1235624421742034243274200250898126767144663061557465289675958910652abcdefghijklmnopqrstuvwxyzABC你好";
	//unsigned char input[]="据台湾媒体报道，苹果iPhone 6发布进入倒计时，其主要代工厂富士康位于郑州、深圳、太原等厂区正在进展加工出货。鸿海集团（富士康母公司）董事长郭台铭近期多次前往iPhone 6生产线坐镇，务求首批7000万至8000万部产品顺利交货。由于iPhone 6新机屏幕尺寸可能加大，分析称会有助于提升手机用户的购买欲望，估计今年苹果的iPhone 6总产能将达到1.2亿部，是历史上出货最大的iPhone系列。";
	//unsigned char input[]="Hacking technique of which (to my suprise) even many security-related people haven't heard of. \nThat is probably because nobody ever really talked about it before.";
	unsigned char input[]="# Concertino\n \n"
	"* polarssl\n"
	"* str.c\n"
	"* path.c\n"
	"* file.c\n"
	"* cc.c\n"
	"* encrypt.c\n"
	"* polarssl\n"
	"* str.c\n"
	"* path.c\n"
	"* file.c\n"
	"* cc.c\n"
	"* encrypt.c\n"
	"* polarssl\n"
	"* str.c\n"
	"* file.c\n"
	"* cc.c\n"
	"* path.c\n"
	"* str.c\n"
	"* encrrypt\n";
	unsigned char encrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	size_t encrypt_length=secrecy_aes(AES_ENCRYPT,
			key,
			input,
			strlen((char *)input),
			encrypt);
	printfln("encrypt:%ld:%s",encrypt_length,encrypt);

	char output[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	string_to_codes(encrypt,encrypt_length,output);
	printfln("output:%s",output);
	
	unsigned char base64_output[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	size_t base64_length=SECRECY_BUFFER_LONG_LENGTH;
	base64_encode(base64_output,&base64_length,encrypt,encrypt_length);
	printfln("base64:%s",base64_output);

	unsigned char block_output[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	_secrecy_make_block("AES-CBC-256",output,(char *)block_output);
	printfln("%s",block_output);

	unsigned char hash_output[20]={'\0'};
	sha1(input,strlen((char *)input),hash_output);
	//printfln("hash:%s",hash_output);
	unsigned char hash_base64[100]={'\0'};
	size_t hash_base64_length=100;
	base64_encode(hash_base64,&hash_base64_length,hash_output,20);
	printfln("hash_base64:%s",hash_base64);

	printfln("----------\n");
	unsigned char decrypt[SECRECY_BUFFER_LONG_LENGTH]={'\0'};
	size_t decrypt_length=secrecy_aes(AES_DECRYPT,
			key,
			encrypt,
			encrypt_length,
			decrypt);
	printfln("decrypt:%ld:%s",decrypt_length,decrypt);
}
void test_secrecy_get_block_content(){
	char block[]=
		"---------AES-CBC-256---------\n"
"ZIdOSoTNDPJyfZpEXiOj1TpD/VnZUSYkVOJyrtnXOyavQJ5eOgtzjF2TI/TvaJ2vnDM0j0zPYKL6EKbczQXbwJdd6ND3bUJXFuYWXtU6y5CrBSdNqeCt2pwF4+vK1B79ZH92NTeQMveJ1CAoJYWGYBUo236qvaTrLADWsczLMm0yPAGzzEvJ6LIH2KrzqCy6PfGKLCKgmodcpxg09jEwuiilklN2dKviv7BSYzyGH8ueV7TWHJRZQWW564UKc/fh";
	char block_content[1000]={'\0'};
	_secrecy_get_block_content(block,"AES-CBC-256",block_content);
	printf("%s\n",block_content);
}
void testcodes_to_string(){
	char input[]="100 135 78 74 132 205 12 242 114 125 154 68 94 35 163 213 58 67 253 89 217 81 38 36 84 226 114 174 217 215 59 38 175 64 158 94 58 11 115 140 93 147 35 244 239 104 157 175 156 51 52 143 76 207 96 162 250 16 166 220 205 5 219 192 151 93 232 208 247 109 66 87 22 230 22 94 213 58 203 144 171 5 39 77 169 224 173 218 156 5 227 235 202 212 30 253 100 127 118 53 55 144 50 247 137 212 32 40 37 133 134 96 21 40 219 126 170 189 164 235 44 0 214 177 204 203 50 109 50 60 1 179 204 75 201 232 178 7 216 170 243 168 44 186 61 241 138 44 34 160 154 135 92 167 24 52 246 49 48 186 40 165 146 83 118 116 171 226 191 176 82 99 60 134 31 203 158 87 180 214 28 148 89 65 101 185 235 133 10 115 247 225";
	char output[1000]={'\0'};
	size_t output_length=codes_to_string(input,output);
	printf("output:%ld,%s\n",output_length,output);

	char printable_string[1000]={'\0'};
	string_to_codes((unsigned char *)output,
			output_length,
			printable_string);
	printf("printable_string:%s\n",printable_string);
}
void test_encrypt_file(){
	unsigned char key[]=SECRECY_TEST_SECRET_KEY;
	secrecy_encrypt_file(SECRECY_TEST_FILE,SECRECY_TEST_FILE,key,"secrecy test secret key");
	//secrecy_encrypt_file("./cppp3.md","./cppp3.secrecy.md",key);
}
void test_decrypt_file(){
	unsigned char key[]=SECRECY_TEST_SECRET_KEY;
	secrecy_decrypt_file(SECRECY_TEST_FILE,SECRECY_TEST_FILE,key,1);
	//secrecy_decrypt_file("./cppp3.secrecy.md","./cppp3.decrypt.md",key,1);
}
void secrecy_self_test(){
	//test_secrecy_aes();
	//test_polarssl_aes();
	//test_file_write();
	//test_secrecy_get_block_content();
	//testcodes_to_string();
	test_encrypt_file();
	test_decrypt_file();
}
/*
int main(int arg_c,char *arg_v[]){
	secrecy_self_test();	
	return 0;
}
*/
