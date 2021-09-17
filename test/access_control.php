<?php

	class A {
		private $money = 10000;
		public function doSth($anotherA) {
			$anotherA->money = 10000000000;//very bad. but in php, it's a valid code
		}

		public function getMoney() {
			return $this->money;    
		}
	}

	$b = new A();
	echo $b->getMoney(); // 10000

	$a = new A();
	$a->doSth($b);
	echo $b->getMoney(); // 10000000000;
