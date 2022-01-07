# lwhttpd
## 简述
### 此项目的由来
作者在学习HTTP的过程中偶然了解到tinyhttpd项目<http://tinyhttpd.sourceforge.net/>，遗憾的是该项目已经超过20年没有更新。  
在这20年里，计算机和互联网络发生了翻天覆地的变化，诞生了很多新的技术，比如IPv6, FastCGI, C++11, epoll...同时有的技术已经过时，为  
此作者决定自己写一个最简单的http服务程序。本项目在编写时借鉴了tinyhttpd, **但与tinyhttpd没有任何关系。**
### 特点
为了使程序尽可能的简单，但又不失学习价值，本项目仅支持最简单的HTTP/1.0 GET和POST方法。  
并未支持TLS，尽管TLS已经普遍使用并且必不可少。  
也没有使用效率更高的FastCGI代替CGI，尽管CGI的fork-and-exec效率饱受诟病。  
也没有使用多线程和多进程，因为那样会是代码更加复杂。  
虽然使用了EPOLL，但是考虑到代码复杂性，我还是选择了电平触发，而非更高效的边沿触发。  
总结如下：  
- 仅支持GET和POST两种method
- 支持静态html和CGI
- 使用了EPOLL
- 同时支持IPv4和IPv6(相同端口)
- 使用CMake构建，仅支持Linux
- 单线程
- 包含一个CGI示例程序
- 默认同时支持IPv4和IPv6
- 支持通过JSON配置
## 警告
本项目仅供学习用途，不提供任何保障。另外，为了使程序尽可能的简单，本程序可能会存在效率地下和安全漏洞，意味着你不可以用于生产用途。  
## 开始
### 准备
为了编译和运行本程序，你需要一下条件:  
- 一台具有root或sudo权限的Linux计算机
- 支持C++11的C++编译器
- CMake 3.0及以上版本(我没试过更低版本)
- BOOST 1.75及以上版本
### 编译
```shell script
# Debug version
$ cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug
# Release version(default)
$ cmake -S . -B ./build
$ cd build/
$ make -j8
```
### 配置
现已支持通过JSON配置，这样就不用每次修改配置都要重新编译代码了  
(注意: TLS还未支持)
```json
{
  "serverName": "localhost",
  "listen": [
	{
	  "address": "0.0.0.0",
	  "port": 80
	},
	{
	  "address": "::",
	  "port": 80
	}
  ],
  "webRoot": "/var/htdocs",
  "tlsConfig": {
	"tlsCertFile": "/cert/tls.cert",
	"tlsKeyFile": "/cert/tls.key"
  }
}
```
### 运行
```shell script
# Make the 'htdocs' directory, and copy demo.cgi to the directory
build$ tree
|-- htdocs
|   `-- demo.cgi
`-- lwhttpd
build$ sudo ./lwhttpd
```
### 测试
#### 1.CURL
```shell script
# IPv4
$curl -v http://127.0.0.1:8086
$curl -v http://127.0.0.1:8086/demo.cgi?hello
# IPv6
$curl -v http://::1:8086
$curl -v http://::1:8086/demo.cgi?hello
```
#### 2.浏览器
浏览器直接访问
