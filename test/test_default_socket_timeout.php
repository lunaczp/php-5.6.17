<?php
	error_reporting(-1);
	ini_set("display_errors", "1");
	$old = ini_set("default_socket_timeout", 0);
	echo $old;
	$redis = new \Redis();
	$a = $redis->connect("127.0.0.1", 6379,2);
	var_dump($a);

	function t() {
		global $redis;
		try {
			$a  = $redis->get("a");
			var_export($a);
		} catch (\RedisException $e) {
			var_dump($e);
		}
	}

	$i = 0;
	$step = 5;
	while (1) {
		t();
//		echo ($i+=$step);
		sleep($step);
	}

