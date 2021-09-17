# class access control in php
php的类的访问控制有很多瑕疵。  
* 比如，可以通过实例访问静态方法；可以通过类访问实例方法
```
<?php
 
error_reporting(E_ALL);
 
class A {
    public static function staticFunc() {
        echo "static";
    }
 
    public function instanceFunc() {
        echo "instance";    
    }
}
 
A::instanceFunc(); // instance
$a = new A();
$a->staticFunc();  // static
```

* 同一个类的两个实例，可以相互修改其私有成员（检验不彻底）
```
<?php
 
class A {
    private $money = 10000;
    public function doSth($anotherA) {
        $anotherA->money = 10000000000;
    }
 
    public function getMoney() {
        return $this->money;    
    }
}
 
$b = new A();
echo $b->getMoney(); // 10000
 
$a = new A();
$a->doSth($b);
echo $b->getMoney(); // 10000000000;
```

so sad...

## ref
[tipi](http://www.php-internals.com/book/?p=chapt05/05-03-class-visibility)
