<?php
	$a=10;
	$x=20;
	$b=&$a;
	$a=&$x;

	xdebug_debug_zval('a');
	xdebug_debug_zval('b');
	xdebug_debug_zval('x');
