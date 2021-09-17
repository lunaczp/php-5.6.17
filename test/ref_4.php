<?php
	$a=10;
	$b=&$a;

	$x=20;
	$a=&$x;
	echo $a.PHP_EOL;
	echo $b.PHP_EOL;
	echo $x.PHP_EOL;
	xdebug_debug_zval('a');
	xdebug_debug_zval('b');
	xdebug_debug_zval('x');
