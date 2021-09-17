# zend lex and bison 解析

## token的产生
* lex产生一个token
    * token值定义在zend_language_parse.c:295
* bison转换这个token到bison新的值（对应表zend_language_parser.c:677）
* bison的token值对应的名称在结构体yytname zend_language_parser.c:1052
    * 其原始定义在zend_language_parse.y中，通过%token定义(c文件是bison生成的)

例如，for
* lex码323 zend_language_parser.c:362
```
#define T_FOR 323
```
* bison码87 zend_language_parser.c:711
* bison名"for (T_FOR)" zend_language_parser.c:1086