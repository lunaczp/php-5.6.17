<?php
	function a() {
		static $x=0;
		$x++;
		echo $x;
	}
	a();
	a();
