# zend gc
php 5.3 之前，一直是用引用计数来做内存管理的。但这个方法有一个问题，会造成内存泄漏。示例代码如下:  
```
<?php
	$a = array( 'one' );
	$a[] =& $a;
	xdebug_debug_zval( 'a' );
?>
//output
a: (refcount=2, is_ref=1)=array (0 => (refcount=1, is_ref=0)='one', 1 => (refcount=2, is_ref=1)=...)
```
对于这种循环引用的情况，当unset($a)的时候，a的计数由2变为1，按照规则是不会被回收的。但实际上我们知道，a已经不会再被使用了，
而且无法在代码中使用a，这种情况，就是发生了内存泄漏（类似于黑客帝国内，被移出系统的特工Smith，内存泄漏了。。。）  
当然这些内存在php进程退出时是会被清理的。但对于一些常驻进程或其他场景，内存的消耗是相当可观的。因而在php 5.3，引入了了新的垃圾回收策略。

新的垃圾回收策略，解决了这个问题，其实就是如何把这些循环引用找出来，并回收之。
php的处理流程如下：
1. 在unset或者销毁一个变量时，尝试把它加入"可能根"缓存列表，并标记为紫色。
2. 在缓存列表满时，执行回收流程（3）
3. 遍历缓存列表，把紫色节点标记为灰色，对其子节点循环遍历，标记灰色并执行refcount--。
4. 遍历缓存列表，对灰色节点，refcount=0则标记白色并加入zval_to_free列表；否则标记黑色并执行refcount++恢复计数。从缓存列表移除该节点。
5. 遍历zval_to_free列表，分别尝试
    1. 调用对象自身的析构函数
    2. 销毁对象
    3. 销售zval
6. 缓存列表已经清空，该回收的也已经回收。完毕（下一个轮回）。

注意，其中第3，第4步骤其实就是一个模拟refcount-1的过程。恰好通过这个过程，把例子中的a给找到了（refcount=0）。具体来讲，a首先进入缓存列表，
第3步，标记a为灰色，并对其子节点执行refcount-1，而其子节点$a[0]恰好是指向a对象自身的，这样，就导致a指向的对象refcount也变成了0。
第4步的时候，就会对a标记为白色，并最终被回收。

## Ref
* [PHP-Manual](http://docs.php.net/manual/en/features.gc.collecting-cycles.php)
* [循环引用计数论文](http://researcher.watson.ibm.com/researcher/files/us-bacon/Bacon01Concurrent.pdf)