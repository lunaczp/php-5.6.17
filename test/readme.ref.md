# zval ref
zend 对变量赋值做了优化，优化了内存使用。

## 基本概念
一个赋值对简单实现例子，
```
$a = 10;
$b = $a;
```
我们可以分别为a和b申请内存空间，但这样其实是浪费，因为a和b对内存是一样的。在zend中，a和b是指向同一个内存空间的。***直到不得不分开时，才申请新的内存空间***。 比如
* 对b赋了新值，且与a不同。
以下为几个例子

[ref_2.php](ref_2.php)
```
<?php
	$a = "string";
	xdebug_debug_zval( 'a' );

	$b = $a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );

	$b = "other";//b有了新值，需要新的内存空间/zval
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );
```
```
a: (refcount=1, is_ref=0)='string'
a: (refcount=2, is_ref=0)='string'
b: (refcount=2, is_ref=0)='string'
a: (refcount=1, is_ref=0)='string'
b: (refcount=1, is_ref=0)='other'
```

[ref.php](ref.php)
```
<?php
	$a = "string";
	xdebug_debug_zval( 'a' );

	$b = $a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );

	$c = &$a;//c成了a的引用，b需要走开了/申请新的内存空间/zval
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );
	xdebug_debug_zval( 'c' );
```
```
a: (refcount=1, is_ref=0)='string'
a: (refcount=2, is_ref=0)='string'
b: (refcount=2, is_ref=0)='string'
a: (refcount=2, is_ref=1)='string'
b: (refcount=1, is_ref=0)='string'
c: (refcount=2, is_ref=1)='string'
```

[ref_3.php](ref_3.php)
```
<?php
	$a = "string";
	xdebug_debug_zval( 'a' );

	$b = &$a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );

	$c = $a;//此种情况，会直接为c分配新的内存空间，不和a b混在一起
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );
	xdebug_debug_zval( 'c' );
```
```
a: (refcount=1, is_ref=0)='string'
a: (refcount=2, is_ref=1)='string'
b: (refcount=2, is_ref=1)='string'
a: (refcount=2, is_ref=1)='string'
b: (refcount=2, is_ref=1)='string'
c: (refcount=1, is_ref=0)='string'
```
对ref.php和ref_3.php，简单一句话概括，当一个变量存在引用的时候，之前以及未来通过$y=$x指向到该变量的变量都需要申请新的内存空间，而不是混在一起。

ps. 相关的结构
* is_f true|false 是否存在引用（&）
* refcount 指向当前zval的个数（$b=$a, $c=&$a, ...）
（如果存在引用，则refcount>=2, 变量本身1个，引用本身1个。反过来，refcount<2，绝不存在引用）

## 其他特性
PHP的&符号，是引用，类似于C中的取地址
```
<?php
	$a=10;
	$x=20;
	$b=&$a;

	$a=&$x;//此时a,b将要分开。a x一个内存空间，b一个内存空间。b并没有和a一起指向x
	echo $a.PHP_EOL;
	echo $b.PHP_EOL;
	echo $x.PHP_EOL;
	xdebug_debug_zval('a');
	xdebug_debug_zval('b');
	xdebug_debug_zval('x');
```
```
20
10
20
a: (refcount=2, is_ref=1)=20
b: (refcount=1, is_ref=0)=10
x: (refcount=2, is_ref=1)=20
```
具体到实现上来讲，如上PHP代码中的a b x都是符号表中的一项，而真正的值，保存在一系列zval中。这两者直类似通过连线对应起来。而$a=&$x，其实就是相当于把
a连接到了x对应的zval。并不会影响b的连线（原来a b连的同一个zval）  
