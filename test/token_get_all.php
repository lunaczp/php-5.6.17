<?php
$code =<<<PHP_CODE
<?php
	\$str = "Hello, Tipi\n";
	echo \$str;
PHP_CODE;
var_dump(token_get_all($code));
