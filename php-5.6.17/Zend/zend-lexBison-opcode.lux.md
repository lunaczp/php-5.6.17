### zend的语法解析，opcode生成，处理流程

1. bison yyparse解析，调用不同的处理函数 zend_language_parser.c:3806
2. 不同的处理函数，产生opcode 如zend_compile.c:872
3. 循环opcode数组，填充handler zend_opcodes.c:752
3. 执行opcode
    1. 循环opcode数组，调用handler zend_vm_execute.h:363
