<?php
	$foo['love'] = 1;
	$bar  = &$foo['love'];
	$tipi = $foo;
	$tipi['love'] = '2';
	echo $foo['love'];
