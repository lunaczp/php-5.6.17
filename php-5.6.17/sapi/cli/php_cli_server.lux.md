# php cli server
php内置简易http server，通过select来实现事件监听。流程：
* select
* 请求事件
    * 静态文件：查找设置返回数据，添加返回事件
    * php文件：解析并设置返回数据，添加返回事件
* 输出事件
    * 输出