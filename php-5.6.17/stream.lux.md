# stream

### 启动注册
* sapi startup(all kinds of sapi startup: `php_cli_startup`, `php_cig_startup`)
* `php_module_startup`
* `php_init_stream_wrappers` (register tcp,udp,...)

### 使用
* 创建 `php_stream_xport_create`(`main/streams/php_stream_transport.h:57`)。  
对于client，创建完成了create, connect操作；对于server，创建完成了create, bind，listen操作
	* 如果提供了`persistent_id`，根据它查找是否存在该资源并有效。是则返回，否则继续。
	* 根据提供的host选择对应的协议类型，并调用对应的工厂方法，工厂方法返回创建好的stream对象。
		* host：`unix:///tmp/a.sock`,则protocal是`unix`，工厂方法：`php_stream_generic_socket_factory`
		* host：`10.7.8.9` 则对应的protocol是`tcp`，工厂方法：`php_stream_generic_socket_factory`
	* 根据是client还是server调用stream方法来`bind`，`listen`等初始化连接
	* 返回stream

* 关于`php_stream_generic_socket_factory`
	* 调用`_php_stream_alloc`初始化一个stream对象
		* new一个php_stream
		* 初始化
		* 注册persisten_id到persistent_list
		* 注册php_stream到regular_list
		* 返回php_stream对象
	* 返回该stream
