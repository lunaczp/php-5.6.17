<?php
	class a {
		public $x = '1';
		private $y = '2';
		protected $z = '3';
		static $s = '4';
		const CON = '5';

		private function privF($p) {
			echo $p;
		}

		public function pubF($p) {
			echo $p;
		}

		protected function proF($p) {
			echo $p;
		}

		public static function staF($p) {
			echo $p;
		}
	}

	$n = new a();
	$n->pubF(1);
	a::staF(1);
