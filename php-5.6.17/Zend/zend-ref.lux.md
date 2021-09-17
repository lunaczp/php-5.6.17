# zend ref 变量及引用

之前提到过zend的refcount计数，如下例子
```
➜  test git:(master) ✗ cat ref_1.php
<?php
	$a="x";
	xdebug_debug_zval('a');
➜  test git:(master) ✗ php ref_1.php
a: (refcount=1, is_ref=0)='x'
```

相关的php源码
```
//zend_execute.c:866
static inline zval* zend_assign_const_to_variable(zval **variable_ptr_ptr, zval *value TSRMLS_DC)
{//lux: $a=10
	zval *variable_ptr = *variable_ptr_ptr;
	zval garbage;

	if (Z_TYPE_P(variable_ptr) == IS_OBJECT &&
	    UNEXPECTED(Z_OBJ_HANDLER_P(variable_ptr, set) != NULL)) {
		Z_OBJ_HANDLER_P(variable_ptr, set)(variable_ptr_ptr, value TSRMLS_CC);
		return variable_ptr;
	}

 	if (UNEXPECTED(Z_REFCOUNT_P(variable_ptr) > 1) &&
 	    EXPECTED(!PZVAL_IS_REF(variable_ptr))) {//$a=10;$b=$a; $a=20; 则为a申请新内存，并赋值20
		/* we need to split */
		Z_DELREF_P(variable_ptr);//lux: 先把原有的zval count 减一（因为a不再指向这个zval了。）
		GC_ZVAL_CHECK_POSSIBLE_ROOT(variable_ptr);
		ALLOC_ZVAL(variable_ptr);
		INIT_PZVAL_COPY(variable_ptr, value);
		zval_copy_ctor(variable_ptr);
		*variable_ptr_ptr = variable_ptr;
		return variable_ptr;
 	} else {
		if (EXPECTED(Z_TYPE_P(variable_ptr) <= IS_BOOL)) {
			/* nothing to destroy */
			ZVAL_COPY_VALUE(variable_ptr, value);
			zendi_zval_copy_ctor(*variable_ptr);
		} else {
			ZVAL_COPY_VALUE(&garbage, variable_ptr);
			ZVAL_COPY_VALUE(variable_ptr, value);
			zendi_zval_copy_ctor(*variable_ptr);
			_zval_dtor_func(&garbage ZEND_FILE_LINE_CC);
		}
		return variable_ptr;
	}
}
```
正常地，会认为代码走到else，因为$a的refcount=0。但实际上，这个地方a的refcount>=2。原因是在调用该函数之前，对a初始化的时候,是直接把executor_globals.uninitialized_zval_prt赋给了a。
从名字即可看出，其是公用的尚未初始化的默认zval。又由于在对a赋值前，会先对uninitialized_zval_ptr的refcount+1
```
//zend_execute.c:279
		Z_ADDREF(EG(uninitialized_zval));
		zend_hash_quick_update(EG(active_symbol_table), cv->name, cv->name_len+1, cv->hash_value, &EG(uninitialized_zval_ptr), sizeof(zval *), (void **)ptr);
```
自然其refcount>=2.
实际上，多次调试的结果显示，在这里+1前，其refcount就=2了，所以初始化后a的refcount=3。
当然，我们看到的xdebug的输出，是在$a=10执行结束之后，此时$a已经完整地初始化过，refcount=1。

### todo
* 什么地方对executor_globals.uninitialized_zval_ptr引用了。
