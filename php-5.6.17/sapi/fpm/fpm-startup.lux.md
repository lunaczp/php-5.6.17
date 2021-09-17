# fpm 启动流程
* 模块启动
```
//fpm_main.c:1807
if (cgi_sapi_module.startup(&cgi_sapi_module) == FAILURE) {//lux:模块启动
```
* fpm_run
    * 根据配置，为各个进程池启动子进程
    * 父进程进入主事件循环
    * 子进程返回，进入处理请求循环
        ```
		    while (fcgi_accept_request(&request) >= 0) {//lux：开始处理请求
        ```
        
* 主事件循环  
事件循环有两类，一类是注册到系统event pool的监听事件。一类是维护的timer事件。
    * 主循环每次首先找到timer事件中的最短事件，然后利用这个事件的timeout时间作为限制，去请求event pool内的事件，有则处理。
    * 然后去遍历timer事件队列，对到期的事件，执行其回调。

举例，注册到event pool的事件比如信号处理。主进程收到信号后，会调用注册的处理函数
```
//fpm_signal.c:156
static void sig_handler(int signo) /* {{{ */
{
	static const char sig_chars[NSIG + 1] = {
		[SIGTERM] = 'T',
		[SIGINT]  = 'I',
		[SIGUSR1] = '1',
		[SIGUSR2] = '2',
		[SIGQUIT] = 'Q',
		[SIGCHLD] = 'C'
	};
	char s;
	int saved_errno;

	if (fpm_globals.parent_pid != getpid()) {
		/* prevent a signal race condition when child process
			have not set up it's own signal handler yet */
		return;
	}

	saved_errno = errno;
	s = sig_chars[signo];
	write(sp[1], &s, sizeof(s));
	errno = saved_errno;
}
```
把信号加入event，然后在主循环中，通过event pool去拉取事件，调用回调函数
```
//fpm_events.c:56
static void fpm_got_signal(struct fpm_event_s *ev, short which, void *arg) /* {{{ */
{/* lux: signal 事件回调 */
	char c;
	int res, ret;
	int fd = ev->fd;
```
这里的实现，其实是通过建立一个socketpair通信通道。先注册信号处理函数，函数负责把信号事件写入socket通道。然后另一方面把socket的另一端注册到event pool的监听队列，这里负责读事件。  
这样，操作系统发送的信号会直接写到socket内。而主事件循环会通过event pool来不断监听socket来读出信号并处理之。


* 子进程处理请求
```
//fpm_events.c:1891
		while (fcgi_accept_request(&request) >= 0) {//lux：开始处理请求
			char *primary_script = NULL;
			request_body_fd = -1;
```
子进程循环等待请求的到来，fcgi_accept_request会调用accept，请求会被阻塞到这里，直到请求到来，会有一个等待的子进程被唤起。
```
//factcgi.c:152
					req->fd = accept(listen_socket, (struct sockaddr *)&sa, &len);//lux: accept
```
多个子进程监听同一个端口（比如9000），请求的分配依靠系统调用accept来处理。请求到来时，只会有一个进程被唤起（具体的分配策略由操作系统决定）。
