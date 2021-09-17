# zend brief

## 文件一览
* zend_operator.[h|c]
    * 提供了封装好了的加减乘除及位运算，支持简单的整型、浮点型，以及PHP数组等复杂类型。
        * 对于简单的运算，会优先尝试汇编，以提升效率。
    
* zend_builtin_function.[h|c]
    * 实现了php的内置函数，如define, error_reporting, method_exists, debug_print_backtrace,,,

* zend_constants.[h|c]
    * 实现了常量注册。包括PHP内置的常量如E_ERROR，是通过这里注册；另外用户定义的常量，也从这里注册。
    * 常量保存的位置在全局变量executor_global.zend_constants内（是个hashtable）

## 基本定义和实现

* 基本类型
```
//zend.h:581
/* data types */
/* All data types <= IS_BOOL have their constructor/destructors skipped */
#define IS_NULL		0
#define IS_LONG		1
#define IS_DOUBLE	2
#define IS_BOOL		3
#define IS_ARRAY	4
#define IS_OBJECT	5
#define IS_STRING	6
#define IS_RESOURCE	7
#define IS_CONSTANT	8
#define IS_CONSTANT_AST	9
#define IS_CALLABLE	10
```

* zval
```
//zend.h:334
struct _zval_struct {
	/* Variable information */
	zvalue_value value;		/* value */
	zend_uint refcount__gc;
	zend_uchar type;	/* active type */
	zend_uchar is_ref__gc;
};

//zend.h 332
typedef union _zvalue_value {
	long lval;					/* long value */
	double dval;				/* double value */
	struct {
		char *val;
		int len;
	} str;
	HashTable *ht;				/* hash table value */
	zend_object_value obj;
	zend_ast *ast;
} zvalue_value;

//zend_types.h:55
typedef struct _zval_struct zval;
```

* hashtable
```
//zend_hash.h:67
typedef struct _hashtable {
	uint nTableSize;
	uint nTableMask;
	uint nNumOfElements;
	ulong nNextFreeElement;
	Bucket *pInternalPointer;	/* Used for element traversal */
	Bucket *pListHead;
	Bucket *pListTail;
	Bucket **arBuckets;
	dtor_func_t pDestructor;
	zend_bool persistent;
	unsigned char nApplyCount;
	zend_bool bApplyProtection;
#if ZEND_DEBUG
	int inconsistent;
#endif
} HashTable;
```

* 常用宏
```
//zend_compile.h:415
#define EX(element) execute_data.element
```

* 魔术变量的实现
    * 是在词法解析阶段，直接进行替换掉的，如\__LINE\__，\__FILE\__
    
* zend 全局变量的定义
```
    //zend.c:641
    zend_compiler_globals *compiler_globals;
    zend_executor_globals *executor_globals;
```
