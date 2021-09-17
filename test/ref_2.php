<?php
	$a = "string";
	xdebug_debug_zval( 'a' );

	$b = $a;
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );

	$b = "other";
	xdebug_debug_zval( 'a' );
	xdebug_debug_zval( 'b' );
