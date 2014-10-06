# Secrecy

使用 AES-CBC-256 方式对文件进行加密解密。

## 安装

	$ make
	$ sudo make install

安装后在 /usr/bin/secrecy

## 使用
	
	$ cd dir
	$ secrecy encrypt -i <要加密的文件> -o <加密后输出的文件> -k <密钥> -h <提示> #加密文件
	$ secrecy decrypt -i <要解密的文件> -o <解密后输出的文件> -k <密钥> #解密文件

* `-i` 输入文件 (必须)
* `-o` 输出文件，如果忽略 `-o`，将会覆盖输入文件的内容
* `-k` 指定要加密或者解密的密钥(必须)
* `-h` 提示

## 卸载

	$ sudo make uninstall

卸载就是删除 /usr/bin/secrecy 文件


## 文件格式
当文件加密后，文件内容会有几个区块,每个包括: 

	`---------区块名字---------`
	内容

其中`AES-CBC-256` 是加密算法的名字,`SHA1`是用来校验结果正确的签名,`HINTS`是针对密钥的提示.

比如 lipsum.md 文件内容


	
	# Lorem ipsum
	
	Lorem ipsum dolor sit er elit lamet, consectetaur cillium adipisicing pecu, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. Nam liber te conscient to factor tum poen legum odioque civiuda.
	
加密后成为 `$ secrecy encrypt -i lipsum.md -k test -h test`:

	---------AES-CBC-256---------
	58 194 153 165 245 233 233 225 108 44 219 161 151 28 76 202 99 82 143 217 169 68 50 73 46 109 91 139 70 198 209 32 181 216 104 101 245 105 233 210 48 151 207 94 73 156 179 152 59 240 50 54 173 57 33 19 29 60 109 2 175 92 6 249 115 85 170 236 12 105 142 162 150 185 99 104 250 218 171 233 5 111 83 216 156 139 158 60 255 84 188 141 202 87 18 173 59 108 74 61 210 97 36 129 192 218 206 197 21 167 94 163 232 157 85 132 68 154 173 190 55 254 157 76 238 154 217 75 189 120 43 23 40 190 53 110 91 87 59 189 226 114 112 106 78 43 185 219 152 185 86 137 106 140 180 145 172 75 179 21 35 41 94 124 194 181 96 163 28 13 120 233 184 2 172 223 224 216 156 57 232 44 198 200 179 190 55 101 106 102 124 217 172 225 80 231 143 58 174 173 15 229 111 191 89 36 120 31 124 94 172 197 66 100 140 241 241 109 44 31 215 251 105 191 179 204 175 11 103 19 226 154 207 104 3 20 53 163 132 49 104 224 133 48 231 39 41 45 235 210 46 98 238 122 26 165 228 192 100 22 85 155 184 6 96 46 190 238 196 70 199 110 254 201 225 171 131 53 68 222 12 3 111 59 217 108 46 169 94 24 223 57 127 237 151 191 56 116 166 218 137 224 79 145 15 106 213 236 97 63 93 241 34 92 200 161 202 171 39 154 251 84 36 32 191 169 189 123 61 204 175 154 88 161 106 114 20 25 13 228 205 251 133 70 218 14 8 122 244 33 150 158 80 95 90 81 74 3 231 194 74 231 53 100 61 0 83 41 31 70 183 46 81 160 115 44 2 148 107 191 87 37 108 244 35 169 23 57 227 193 174 207 154 247 156 43 95 107 10 204 212 63 77 150 49 4 61 232 88 14 171 161 239 169 84 174 147 82 202 14 83 132 178 27 171 125 192 252 153 37 106 203 150 43 98 206 99 119 116 253 247 119 219 105 131 150 34 193 166 226 155 37 122 78 140 85 242 1 187 51 250 204 42 57 11 149 165 164 161 82 84 31 194 28 60 99 81 159 142 137 156 163 56 104 79 120 120 34 210 171 109 164 132 68 91 90 47 86 81 220 30 4 132 142 199 191 62 92 18 111 33 42 233 242 20 176 144 85 175 134 28 215 179 72 105 232 72 12 124 90 58 112 234 58 157 247 145 231 4 171 45 121 134 225 
	---------SHA---------
	8B+EPBmCt2rMBRyVpqMjNnh+7Uo=
	---------HINTS---------
	test
	
