# zend opcode

## 定义
* opcode定义(150个) zend_vm_opcodes.h
* opcode handler（4201个） zend_vm_execute.h:41382
    * 167\*25 +5\*5 +5 =4205(opcode最大编码167。每个opcode两个参数，每个参数5种可能(IS_CONST,IS_TMP_VAR,IS_VAR,IS_UNUSED,IS_CV))
* zend_vm_get_opcode_handler:根据opcode及参数获取对应的handler函数

## 常用宏
* 下一条指令
```
#define ZEND_VM_NEXT_OPCODE() \ //zend_execute.c:1772
	CHECK_SYMBOL_TABLES() \
	ZEND_VM_INC_OPCODE(); \
	ZEND_VM_CONTINUE()
	
#define ZEND_VM_INC_OPCODE() \ //zend_execute.c:1789
	OPLINE++
	
#define OPLINE EX(opline) //zend_vm_execute.h:317
```