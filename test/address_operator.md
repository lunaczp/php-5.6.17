# &操作符 写时复制 引用计数 引用污染
看下面的例子，&操作符污染了变量。
```
<?php
	$foo['love'] = 1;
	$bar  = &$foo['love'];
	$tipi = $foo;
	$tipi['love'] = '2';
	echo $foo['love'];//2
```

## ref
[tipi](http://www.php-internals.com/book/?p=chapt06/06-06-copy-on-write)
