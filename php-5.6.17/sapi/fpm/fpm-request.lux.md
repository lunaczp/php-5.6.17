# fpm 请求处理流程
子进程负责接收并处理请求。入口在
```
//fpm_main.c:1891
while (fcgi_accept_request(&request) >= 0) {//lux：开始处理请求
```
fcgi_accept_request
* 调用accept函数来等待链接到来（bind和listen已经在主进程内完成）。
```
//fastcgi.c:852
req->fd = accept(listen_socket, (struct sockaddr *)&sa, &len);//lux: accept
```
* 解析fastcgi请求数据，生成request信息
* 返回进入fpm_main的处理流程:
* php_request_startup
    * zend_active
    * sapi_active
    * php_hash_environment 填充全局变量_GET _POST _FILES
* 一些校验
    * 文件是否存在
    * 文件扩展是否合法
    * 是否读文件
* php_execute_script
* php_request_shutdown
* 如果没有达到请求数限制，则循环。否则执行退出流程:
    * fcgi_shutdown
    * php_module_shutdown
