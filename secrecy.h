#ifndef SECRECY_H
#define SECRECY_H
extern int secrecy_encrypt_file(const char *input_filename,
		const char *output_filename,
		const unsigned char *key,
		const char *hints
		);
extern int secrecy_decrypt_file(const char *input_filename,
		const char *output_filename,
		const unsigned char *key,
		int check_sha
		);
extern void secrecy_self_test();
#endif
