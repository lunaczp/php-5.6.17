<?php
	$a = "string";
	xdebug_debug_zval( 'a' );

	$b = $a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );

	$c = &$a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );
	xdebug_debug_zval( 'c' );
