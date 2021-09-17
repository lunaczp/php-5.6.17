# php extension 机制
php的extension机制是如何实现的呢。只是在ext目录里生成新的文件，就能编译到php的源码内。不合理。查看模块相关的文件，发现
```
// main/internal_functions_cli.c:22

#include "ext/date/php_date.h"
#include "ext/ereg/php_ereg.h"
#include "ext/pcre/php_pcre.h"
#include "ext/myfunctions/php_myfunctions.h"
#include "ext/reflection/php_reflection.h"
#include "ext/spl/php_spl.h"
#include "ext/standard/php_standard.h"
```
新加的模块 myfunction 被自动加入到代码里啦。由此猜测main/internal_functions_cli.c是动态生成的（根据configure的配置和参数，确定哪些模块是需要被加入的。）
再次想到该文件确实是在.gitignore文件中的（动态生成，不加入git）。
```
// .gitignore:231

main/build-defs.h
main/internal_functions_cli.c
```