<?php
	class Foo
	{
		public $pub = '3.14159265359';
		private $pri = '1111111';
		static $c = 0;
		const T_VAR = 'xxxx';

		public function __destruct() {
			echo ++self::$c.PHP_EOL;
		}
	}

	$baseMemory = memory_get_usage();

	for ( $i = 0; $i <= 100000; $i++ )
	{
		$a = new Foo;
		$a->self = $a;
		if ( $i % 500 === 0 )
		{
			echo sprintf( '%8d: ', $i ), memory_get_usage() - $baseMemory, "\n";
		}
	}
?>
