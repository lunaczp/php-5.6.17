<?php
	function a(){
		b();
	}
	function b() {
	debug_print_backtrace();
//var_dump(debug_backtrace());
	}

	a();


