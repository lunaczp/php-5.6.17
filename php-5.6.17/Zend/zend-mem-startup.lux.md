# zend MM 启动流程

php_module_startup -->  zend_startup --> start_memory_manager --> alloc_globals_ctor --> zend_mm_startup --> zend_mm_startup_ex

* 首先，各个sapi会调用php_module_startup来启动自己按规则已经注册好的模块。
* php_module_startup会先调用zend_startup来启动zend，然后做初始化、模块加载等等。
* zend_startup会先调用start_memory_manager启动MM模块，然后做其他初始化。
* start_memory_manager直接调用alloc_globals_ctor
* alloc_globals_ctor会判断USE_ZEND_ALLOC来确定是否使用zend MM，如果是则调用zend_mm_startup
```
//zend_alloc.c:2737
static void alloc_globals_ctor(zend_alloc_globals *alloc_globals TSRMLS_DC)
{
	char *tmp = getenv("USE_ZEND_ALLOC");

	if (tmp && !zend_atoi(tmp, 0)) {//lux: 禁用zend mm(如果配置了环境变量 USE_ZEND_ALLOC)
		alloc_globals->mm_heap = malloc(sizeof(struct _zend_mm_heap));
		memset(alloc_globals->mm_heap, 0, sizeof(struct _zend_mm_heap));
		alloc_globals->mm_heap->use_zend_alloc = 0;
		alloc_globals->mm_heap->_malloc = malloc;
		alloc_globals->mm_heap->_free = free;
		alloc_globals->mm_heap->_realloc = realloc;
	} else {//lux: 使用zend mm（默认）
		alloc_globals->mm_heap = zend_mm_startup();//lux: 初始化内存堆
	}
}
```
* zend_mm_startup会根据ZEND_MM_MEM_TYPE来选取一种分配策略，然后调用zend_mm_startup_ex
```
//zend_alloc.c:311 4种分配策略
 static const zend_mm_mem_handlers mem_handlers[] = {
 #ifdef HAVE_MEM_WIN32
 	ZEND_MM_MEM_WIN32_DSC,
 #endif
 #ifdef HAVE_MEM_MALLOC
 	ZEND_MM_MEM_MALLOC_DSC,
 #endif
 #ifdef HAVE_MEM_MMAP_ANON
 	ZEND_MM_MEM_MMAP_ANON_DSC,
 #endif
 #ifdef HAVE_MEM_MMAP_ZERO
 	ZEND_MM_MEM_MMAP_ZERO_DSC,
 #endif
 	{NULL, NULL, NULL, NULL, NULL, NULL}
 };
```
* zend_mm_startup_ex会调用选择的分配策略(比如win32，调用HeapCreate；malloc，调用malloc)真正分配内存，然后调用zend_mm_init初始化空闲块；然后初始化其他字段
* zend_mm_init初始化alloc_globals->mm_heap的free_buckets,large_free_buckets和rest_buckets
```
//zend_alloc.c:915
	p = ZEND_MM_SMALL_FREE_BUCKET(heap, 0);
	for (i = 0; i < ZEND_MM_NUM_BUCKETS; i++) {
		p->next_free_block = p;
		p->prev_free_block = p;
		p = (zend_mm_free_block*)((char*)p + sizeof(zend_mm_free_block*) * 2);
		heap->large_free_buckets[i] = NULL;
	}
	heap->rest_buckets[0] = heap->rest_buckets[1] = ZEND_MM_REST_BUCKET(heap);
	heap->rest_count = 0;
}
```
* 启动完毕，对应的全部变量为alloc_globals->mm_heap
