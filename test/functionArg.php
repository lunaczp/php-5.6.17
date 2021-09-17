<?php
	function a(integer $x){//error, type hint only support object and array
		echo $x;
	}

	a(3);
