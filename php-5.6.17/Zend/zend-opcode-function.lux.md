# zend function

zend 会对每一个函数生成一个opcode array，并维护一个全局的active_op_array。当发生函数调用时，就切换到被调用函数，从而实现函数调用，和作用域切换。
* 实际上，顶层代码（不在任何函数内）也算是一个函数，只不过函数名是空而已。

vld对比
```
➜  test git:(master) ✗ cat echo.php
<?php
	echo 1;
➜  test git:(master) ✗ php5617vld echo.php
Finding entry points
Branch analysis from position: 0
Jump found. (Code = 62) Position 1 = -2
filename:       /code/testPhpSrc/test/echo.php
function name:  (null)
number of ops:  2
compiled vars:  none
line     #* E I O op                           fetch          ext  return  operands
-------------------------------------------------------------------------------------
   2     0  E >   ECHO                                                     1
   3     1      > RETURN                                                   1

branch: #  0; line:     2-    3; sop:     0; eop:     1; out1:  -2
path #1: 0,
1%
```
```
➜  test git:(master) ✗ cat function.php
<?php
	function a() {
		echo 1;
	}
	a();
➜  test git:(master) ✗ php5617vld function.php
Finding entry points
Branch analysis from position: 0
Jump found. (Code = 62) Position 1 = -2
filename:       /code/testPhpSrc/test/function.php
function name:  (null)
number of ops:  3
compiled vars:  none
line     #* E I O op                           fetch          ext  return  operands
-------------------------------------------------------------------------------------
   2     0  E >   NOP
   5     1        DO_FCALL                                      0          'a'
   6     2      > RETURN                                                   1

branch: #  0; line:     2-    6; sop:     0; eop:     2; out1:  -2
path #1: 0,
Function a:
Finding entry points
Branch analysis from position: 0
Jump found. (Code = 62) Position 1 = -2
filename:       /code/testPhpSrc/test/function.php
function name:  a
number of ops:  2
compiled vars:  none
line     #* E I O op                           fetch          ext  return  operands
-------------------------------------------------------------------------------------
   3     0  E >   ECHO                                                     1
   4     1      > RETURN                                                   null

branch: #  0; line:     3-    4; sop:     0; eop:     1; out1:  -2
path #1: 0,
End of function a

1%
```