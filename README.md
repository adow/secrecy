# Secrecy

使用 AES-CBC-256 方式进行加密解密。

## 安装

	$ make
	$ sudo make install

安装后在 /usr/bin/secrecy

## 使用
	
	$ cd dir
	$ secrecy encrypt -i 要加密的文件 -o 加密后输出的文件 -k 密钥 #加密文件
	$ secrecy decrypt -i 要解密的文件 -o 解密后输出的文件 -k 密钥 #解密文件

* `-i` 输入文件 (必须)
* `-o` 输出文件，如果忽略 `-o`，将会覆盖输入文件的内容
* `-k` 指定要加密或者解密的密钥(必须)

## 卸载

	$ sudo make uninstall

卸载就是删除 /usr/bin/secrecy 文件
