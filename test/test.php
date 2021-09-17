<?php
		$a = "Hello";
		$b = &$a;
		unset($b);
		$b = "World";
		echo $a;
		/**
	function t1() {
		$a = "Hello";
		$b = &$a;
		$b = "World";
		echo $a;
	}

	function t2() {
		$a = "Hello";
		$b = &$a;
		unset($b);
		$b = "World";
		echo $a;
	}

	t1();
	t2();
	**/
