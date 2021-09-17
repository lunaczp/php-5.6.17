# zend function 实现

## 理论
函数语法的支持，要有几点
* 声明
* 实现
* 使用／调用

这是对C语言来说。对PHP来说，其实只有定义（声明+实现）、使用两个地方。

在定义的时候，我们要记录
* 函数的名字
* 函数的参数
* 实现代码，以及入口位置。

在使用的时候，
* 我们通过名字和参数，来定位这个函数。
* 将参数传递给他，跳到入口位置。
* 执行之。
* 返回当前作用域，结束。
当然，执行前后有一些初始化和作用域切换工作要做的。

## PHP实现
下面我们看下PHP的函数是如何实现的。
### 语法解析、词法解析
```
//zend_language_parser.y:238
	|	function_declaration_statement	{ zend_verify_namespace(TSRMLS_C); zend_do_early_binding(TSRMLS_C); }
	
//zend_language_parser.y:413
unticked_function_declaration_statement: //lux: 如 function a() {echo 1;} 三部分构成
		function is_reference T_STRING { zend_do_begin_function_declaration(&$1, &$3, 0, $2.op_type, NULL TSRMLS_CC); }//lux: is_reference 是因为php支持return by reference. 如：function &a() {$x=1; return $x;}
		'(' parameter_list ')'
		'{' inner_statement_list '}' { zend_do_end_function_declaration(&$1 TSRMLS_CC); }
		
//zend_language_parser.y:564
parameter:
		optional_class_type is_reference is_variadic T_VARIABLE
			{ zend_do_receive_param(ZEND_RECV, &$4, NULL, &$1, $2.op_type, $3.op_type TSRMLS_CC); }
	|	optional_class_type is_reference is_variadic T_VARIABLE '=' static_scalar
			{ zend_do_receive_param(ZEND_RECV_INIT, &$4, &$6, &$1, $2.op_type, $3.op_type TSRMLS_CC); }
```
举例，
```
➜  test git:(master) ✗ cat function.php
<?php
	function a() {
		echo 1;
	}
	a();
```
当解析器解析到function a, 发现这是一个函数定义(其实词法解析是要匹配到左括号(，才暂时停止))，调用zend_do_begin_function_declaration，
该函数会处理类的方法和普通函数两类情况，这里只讨论函数的情况。
* 初始化一个zend_op_array，并填充相关信息。
* 生成一个定义函数的zend_op，加入到CG(active_op_array)。
* 保存原有CG(active_op_array)，并将新的op_array赋之。（那么后面函数体内所有产生的opcode都会加入到这个函数对应的op_array）。
* 将函数加入CG(function_table)，其值为构造的zend_op_array。
* 初始化一个context，将CG(context)压栈，然后将新值赋之。
* 压栈一个原始的zend_switch_entry入CG(switch_cond_stack)
* 压栈一个原始的zend_op入CG(foreach_copy_stack)
* 结束。


当解析到不为空的参数时，会调用zend_do_receive_param。这里不涉及，掠过。  
当解析到}时，函数体结束，会调用zend_do_end_function_declaration，
* context恢复
* active_op_array恢复
* CG(switch_cond_stack)恢复
* CG(foreach_copy_stack)恢复
* 结束

之后解析器执行shift/reduce, 然后调用zend_verify_namespace，zend_do_early_binding。下面说下zend_do_early_binding的流程。  
其作用可以叫做早期绑定／编译器绑定。实现上，根据类型分别调用
* do_bind_function
* do_bind_class
* ...  
这里说下do_bind_function，
* 从CG(active_op_array)中拿到最后一条定义函数的zend_op
* 从zend_op中，解析出op1(保存了函数的具体实现对应的op array)，op2(保存了函数名：a)；
* 将a加入CG(function_table)，其值为解析出的op array（拷贝）。
* 清理
    * 清理中间变量
    * 重置这条函数定义的zend_op为NOP（因为函数定义已经处理完了。后面最终执行op array时，就不再处理了。）
* 结束

### 函数调用
解析到函数调用时（a），调用zend_do_begin_function_call处理，其流程如下：
* 解析名称。
* 尝试zend_do_begin_dynamic_function_call。（类相关）
* 在CG(function_table) 查找该函数。如果找不到，调用zend_do_begin_dynamic_function_call。
* 找到。将该函数加入调用栈zend_push_function_call_entry
* 调用zend_do_extended_fcall_begin
    * 根据编译配置，如果需要更多信息则:
        * 生成一条zend_op，标记为ZEND_EXT_FCALL_BEGIN，填充。
    * 返回
* 结束。  

解析器碰到参数时，会进行参数处理，这里没有，略过。  
解析器遇到}是，调用zend_do_end_function_call，流程如下：
* 从CG(function_call_stack)取头部，拿到function
* 生成zend_op，标记ZEND_DO_FCALL，填充其他，
* 处理完毕，CG(function_call_stack)出栈。
* 结束。
然后会调用zend_do_extended_fcall_end
    * 根据编译配置，如果需要更多信息
        * 生成zend_op，标记为ZEND_EXT_FCALL_END，填充其他。
        * 返回
* 结束。



