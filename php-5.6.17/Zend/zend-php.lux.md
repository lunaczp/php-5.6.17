# zend 对php的interface（暴露的函数、变量类型）

## 变量
### PHP的集中变量类型
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

## function
zend引擎会在初始化的时候，加载PHP内部函数(如print_f)到函数表compile_globals.function_table，这样在程序执行的时候，可以直接查找并使用。  
注意这类并不是语法结构（类似try catch 是语法结构）,而只是暴露的函数，可在PHP中直接调用。  
实际上，你并不会在词法解析的时候发现针对print_f等内部函数的特殊处理，他们和用户定义好的函数一样通过函数结构被识别，而不是通过关键字本身（因而可以说print_f不是PHP关键字）。
