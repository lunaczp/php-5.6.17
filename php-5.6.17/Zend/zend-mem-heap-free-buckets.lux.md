#free_buckets

## 初始化free_buckets
```
//zend_alloc.c:908
	p = ZEND_MM_SMALL_FREE_BUCKET(heap, 0);
	for (i = 0; i < ZEND_MM_NUM_BUCKETS; i++) {
		p->next_free_block = p;
		p->prev_free_block = p;
		p = (zend_mm_free_block*)((char*)p + sizeof(zend_mm_free_block*) * 2);
		heap->large_free_buckets[i] = NULL;
	}
	
//zend_alloc.c:453
#define ZEND_MM_SMALL_FREE_BUCKET(heap, index) \
	(zend_mm_free_block*) ((char*)&heap->free_buckets[index * 2] + \
		sizeof(zend_mm_free_block*) * 2 - \
		sizeof(zend_mm_small_free_block))
		
//zend_alloc.c:415
struct _zend_mm_heap {
	int                 use_zend_alloc;
	void               *(*_malloc)(size_t);
	void                (*_free)(void*);
	void               *(*_realloc)(void*, size_t);
	size_t              free_bitmap;
	size_t              large_free_bitmap;
	size_t              block_size;
	size_t              compact_size;
	zend_mm_segment    *segments_list;
	zend_mm_storage    *storage;
	size_t              real_size;
	size_t              real_peak;
	size_t              limit;
	size_t              size;
	size_t              peak;
	size_t              reserve_size;
	void               *reserve;
	int                 overflow;
	int                 internal;
#if ZEND_MM_CACHE
	unsigned int        cached;
	zend_mm_free_block *cache[ZEND_MM_NUM_BUCKETS];
#endif
	zend_mm_free_block *free_buckets[ZEND_MM_NUM_BUCKETS*2];
	zend_mm_free_block *large_free_buckets[ZEND_MM_NUM_BUCKETS];
	zend_mm_free_block *rest_buckets[2];
	int                 rest_count;
#if ZEND_MM_CACHE_STAT
	struct {
		int count;
		int max_count;
		int hit;
		int miss;
	} cache_stat[ZEND_MM_NUM_BUCKETS+1];
#endif
};
```

可看出，heap->free_buckets是个数组，存储指向zend_mm_free_block的指针。正常初始化是遍历数据就行。但这里采用了另外的操作。
其关键点在于ZEND_MM_SMALL_FREE_BUCKET宏，看了很久看不出来含义是什么。最后发现，它是利用了内存布局，强制／巧妙构造一个zend_mm_free_block对象，然后
通过
```
p->next_free_block = p;
p->prev_free_block = p;
```
来初始化。

比如，现在heap->free_buckets的内存地址和内容是
```
(lldb) p &heap->free_buckets
(zend_mm_free_block *(*)[128]) $168 = 0x00007fd33d800298

(lldb) p heap->free_buckets
(zend_mm_free_block *[128]) $167 = {
  [0] = 0x00007fd33d800278
  [1] = 0x00007fd33d800278
  [2] = 0x00007fd33d800288
  [3] = 0x00007fd33d800288
  [4] = 0x0000000000000000
  [5] = 0x0000000000000000
  [6] = 0x0000000000000000
  [7] = 0x0000000000000000

```
显然，我已经初始化好了前面2对（index:0,1,2,3），现在i=2，我要初始化第4，5个。代码是
```
p->next_free_block = p;
p->prev_free_block = p;
```
4，5明明是数组下标，跟p完全没关系，为什么对p对赋值能影响4，5的内存呢。原因只有一个就是，对p的这个赋值操作，修改了4，5对应对内存。
直白的讲，他们对应的内存地址是同一个，也就是p->next_free_block 和5是同一个内存空间，为什么。我们看下p
```
(lldb) p p
(zend_mm_free_block *) $169 = 0x00007fd33d800298
```
p是指向0x00007fd33d800298的zend_mm_free_block结构体。其内存分布，就是从0x00007fd33d800298开始的连续的内存空间
```
(lldb) x/12xw 0x00007fd33d800298
0x7fd33d800298: 0x3d800278 0x00007fd3 0x3d800278 0x00007fd3
0x7fd33d8002a8: 0x3d800288 0x00007fd3 0x3d800288 0x00007fd3
0x7fd33d8002b8: 0x00000000 0x00000000 0x00000000 0x00000000
```
当执行p->next_free_block = p;时，就是修改了这块内存内的对应一块，具体是哪块，要看zend_mm_free_block
```
typedef struct _zend_mm_free_block {
	zend_mm_block_info info;
#if ZEND_DEBUG
	unsigned int magic;
# ifdef ZTS
	THREAD_T thread_id;
# endif
#endif
	struct _zend_mm_free_block *prev_free_block;
	struct _zend_mm_free_block *next_free_block;

	struct _zend_mm_free_block **parent;
	struct _zend_mm_free_block *child[2];
} zend_mm_free_block;
```
其中，info占8\*3个字节(指针长度8,size_t长度8)，magic占8\*1个。所以prev_free_block在第5个字块（32字节块)，next_free_block在第六个。
执行第一条赋值语句后
```
(lldb) x/12xw 0x00007fd33d800298
0x7fd33d800298: 0x3d800278 0x00007fd3 0x3d800278 0x00007fd3
0x7fd33d8002a8: 0x3d800288 0x00007fd3 0x3d800288 0x00007fd3
0x7fd33d8002b8: 0x00000000 0x00000000 0x3d800298 0x00007fd3
```
看到第六个字块的内容更新了,就是赋的p的值。

也就是说，__这个宏的目的就是，通过指定p，来使p->prev_free_block，p->next_free_block指向要操作的内存块__。  
如何计算呢，其实就是先找到要修改的内存，比如0x00007fd33d8002b8，然后假设这个内存块对应的是zend_mm_free_block结构内的prev_free_block，然后往前找到zend_mm_free_block
结构体的入口地址／指针（p）。然后通过p->prev_free_block来操作这个地址。

最终回过来看这个宏
```
//zend_alloc.c:453
#define ZEND_MM_SMALL_FREE_BUCKET(heap, index) \
	(zend_mm_free_block*) ((char*)&heap->free_buckets[index * 2] + \
		sizeof(zend_mm_free_block*) * 2 - \
		sizeof(zend_mm_small_free_block))
```
index是我们要修改的数组下标（两个1组，所以0->(0,1), 1->(2,3)）。
返回的是，根据这个下标构造好的假的zend_mm_free_block*（入口地址）。之所以说假的，是因为本身这块内存并不是真的zend_mm_free_block。  
实际上，当index=0的时候，返回的zend_mm_free_block*指向了cache数组
```
	zend_mm_free_block *cache[ZEND_MM_NUM_BUCKETS];
#endif
	zend_mm_free_block *free_buckets[ZEND_MM_NUM_BUCKETS*2];
	zend_mm_free_block *large_free_buckets[ZEND_MM_NUM_BUCKETS];
```
因为要从index=0的内存，往回找。这时的p指向的就是0x00007fd33d800278,是属于cache的内存。

最终初始化的结构
```
(lldb) p &heap->free_buckets
(zend_mm_free_block *(*)[128]) $201 = 0x00007fd33d800298

(lldb) p heap->free_buckets
(zend_mm_free_block *[128]) $211 = {
  [0] = 0x00007fd33d800278
  [1] = 0x00007fd33d800278
  [2] = 0x00007fd33d800288
  [3] = 0x00007fd33d800288
  [4] = 0x00007fd33d800298
  [5] = 0x00007fd33d800298
  [6] = 0x00007fd33d8002a8
  [7] = 0x00007fd33d8002a8
  ...

```
可看到前面4个指向的都是属于cache部分的内存。//todo 这个是否会有问题呢？
