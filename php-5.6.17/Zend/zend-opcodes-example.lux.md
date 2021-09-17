# zend opcode 例子
* for
```
<?php
for ($i=0; $i<3; $i++) {
    echo $i;
}
```
生成的opcode
```
number of ops:  12
compiled vars:  !0 = $i
line     #* E I O op                           fetch          ext  return  operands
-------------------------------------------------------------------------------------
   2     0  E >   EXT_STMT
         1        ASSIGN                                                   !0, 0
         2    >   IS_SMALLER                                       ~1      !0, 3
         3        EXT_STMT
         4      > JMPZNZ                                        8          ~1, ->11
         5    >   POST_INC                                         ~2      !0
         6        FREE                                                     ~2
         7      > JMP                                                      ->2
   3     8    >   EXT_STMT
         9        ECHO                                                     !0
   4    10      > JMP                                                      ->5
   5    11    > > RETURN                                                   1

```