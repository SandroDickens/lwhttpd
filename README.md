# lwhttpd
## 简述
<img src="https://www.lwhttpd.com/assets/lwhttpd.png" alt="LWHTTPD">\
lwhttpd(全称lightweight httpd)是一个轻量级和高性能的web服务器. 支持CGI, FastCGI(计划中), proxy (计划中)和静态资源访问\
本项目主页<https://www.lwhttpd.com>
### 特点
- 支持GET和POST
- 支持静态资源和CGI
- 使用了EPOLL
- 支持IPv4和IPv6
- 使用CMake构建，仅支持Linux
- 使用了线程池
- 包含一个CGI示例程序
- 支持通过JSON配置
### 警告
TLS还未支持，也就是说还不能使用HTTPS
## 开始
### 准备
为了编译和运行本程序，你需要一下条件:  
- 一台Linux计算机(受限于CGI)
- 支持C++17的C++编译器
- 支持pthread线程的编译器
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
已支持通过JSON配置
```json
{
  //域名
  "server_name": "www.mydomain.com",
  //监听地址和端口
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
  //根目录
  "web_root": "/var/htdocs",
  //工作线程池的线程数量, 0默认为CPU可用线程数的2倍
  "work_thread": 0,
  //TLS证书和key
  "tls": {
	"cert_file": "/cert/tls.cert",
	"key_file": "/cert/tls.key"
  }
}
```
### 运行
```shell script
# Make the 'htdocs' directory, and copy demo.cgi to the directory
$ tree
|-- htdocs
|   |-- demo.cgi
|   `-- index.html
`-- lwhttpd
$ sudo ./lwhttpd
```
### 测试
#### 1.CURL
```shell script
# IPv4
$ curl -v http://127.0.0.1
$ curl -v http://127.0.0.1/demo.cgi?hello
# IPv6
$ curl -v http://[::1]
$ curl -v http://[::1]/demo.cgi?hello
# auto
$ curl -v http://localhost/demo.cgi?hello
```
#### 2.浏览器
浏览器直接访问<http://localhost/demo.cgi?hello>
