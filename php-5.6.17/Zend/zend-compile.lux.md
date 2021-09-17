# zend compile CheatList

## compile 流程
* zend_language_parse.[y|c|h] tokenize文件，解析statement，调用对应的处理函数。
* 对应的处理函数在zend_compile.[h|c]中声明及定义。

## 常用变量
* 5种与编译相关的基本变量类型定义 zend_compile.h:423
```
#define IS_CONST	(1<<0) //lux:对应 zend_execute.c _CONST_CODE
#define IS_TMP_VAR	(1<<1)
#define IS_VAR		(1<<2)
#define IS_UNUSED	(1<<3)	/* Unused variable */
#define IS_CV		(1<<4)	/* Compiled variable */
```
* 取临时变量宏
```
#define EX_T(offset) (*EX_TMP_VAR(execute_data, offset)) //zend_execute.c:65
#define EX_TMP_VAR(ex, n)	   ((temp_variable*)(((char*)(ex)) + ((int)(n)))) //zend_compile.h:417
```
* 临时变量的实现 zend_execute.h:30
```
typedef union _temp_variable {
	zval tmp_var;
	struct {
		zval **ptr_ptr;
		zval *ptr;
		zend_bool fcall_returned_reference;
	} var;
	struct {
		zval **ptr_ptr; /* shared with var.ptr_ptr */
		zval *str;
		zend_uint offset;
	} str_offset;
	struct {
		zval **ptr_ptr; /* shared with var.ptr_ptr */
		zval *ptr;      /* shared with var.ptr */
		HashPointer fe_pos;
	} fe;
	zend_class_entry *class_entry;
} temp_variable;

```
