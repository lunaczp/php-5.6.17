# zend 内存分配流程
zend 的MM模块可以认为有三层
* 接口层 emalloc
* 堆层 heap
* 存储层 storage 封装了不同的分配策略
* 操作系统 os
heap层会维护四个内存块链表cache, free_buckets, large_free_buckets, rest_buckets。

## 接口层
```
//zend_alloc.h:69
/* Standard wrapper macros */
#define emalloc(size)						_emalloc((size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define safe_emalloc(nmemb, size, offset)	_safe_emalloc((nmemb), (size), (offset) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define efree(ptr)							_efree((ptr) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define ecalloc(nmemb, size)				_ecalloc((nmemb), (size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define erealloc(ptr, size)					_erealloc((ptr), (size), 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define safe_erealloc(ptr, nmemb, size, offset)	_safe_erealloc((ptr), (nmemb), (size), (offset) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define erealloc_recoverable(ptr, size)		_erealloc((ptr), (size), 1 ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define estrdup(s)							_estrdup((s) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define estrndup(s, length)					_estrndup((s), (length) ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)
#define zend_mem_block_size(ptr)			_zend_mem_block_size((ptr) TSRMLS_CC ZEND_FILE_LINE_CC ZEND_FILE_LINE_EMPTY_CC)

```

## 分配
_emalloc会调用_zend_mm_alloc_int向底层拿到满足需要的内存。_zend_mm_alloc_int:
1. 如果设定了使用缓存，则尝试缓存cache。命中则返回，否则继续
2. 尝试小块内存列表free_buckets，命中则跳转到5，否则继续
3. 尝试大块内存列表large_free_buckets，命中则跳转到5，否则继续
4. 尝试向底层申请新的segment，初始化，挂载到segment链表，并截取生成所需要的内存块, 继续6
5. 将该块标记为不可用，并从可用列表删除，继续6
6. 计算该块内存剩余的部分（拿到的内存块是相对最合适的，可能比需要的大）,并加入到空闲列表
7. 返回拿到的内存块

## 释放
_efree会调用_zend_mm_free_int:
1. 如果设置开启缓存，则尝试加入缓存链表
2. 尝试合并前后空余内存块（从空闲列表移除）
3. 如果合并后，发现是个完整的segment块，则整体删除；否则将合并后的块重新加入空闲列表

## 总结
zend mm 的内存管理的一个特点就是，一次申请，多次使用。具体来讲，mm向操作系统申请内存的时候，会申请一大块：
```
//zend_alloc.c:491
/* Default memory segment size */
#define ZEND_MM_SEG_SIZE   (256 * 1024)
```
在每次使用的时候，会从大块内存内分配出来一块。最终向上层提供的内存池是：
* cache
* free_buckets
* large_free_buckets
* rest_buckets

上层从内存池取用内存；回收的时候再放回来。当内存池不够时，再次向操作系统申请一个大块内存。  
这样，mm就减少了向操作系统的内存申请，减少了系统调用（需要内核态切换，开销大）。  
一个简单的例子就是
```
<?php
	function a() {
		echo 1;
	}
	a();
```
调用了_emalloc 1726次，而底层调用malloc只有1次(php5.6.17)，可见其效果。
