###README

<br/>
####功能
SpxMemoryPool是内存池的实现，它由c写成，不依赖于任何其它库，该pool主要完成了以
下功能：
1. 定义了统一的heap object模型；
2. 定义了pool中的buff功能，并且可以自定义size和pooling object size；
3. 所有的pooling object都memory align;
4. pool可以自动区分large object（默认值为8kb）;
5. memory pool可以被clear，并且可以在不重新分配buffer的时候重复利用；
6. 对于不需要pooling的object，也可以支持；

<br/>
####注意事项
1. 线程不安全，所以需要每个线程一个memory pool，或者自行加锁；
2. 回收object的策略等同于stack，但是如果分配的内存被跳过回收，将不会在pool的该
生命周期内被回收；
3. new pool时，请把memory buffer size设置成远远大于large object size

<br/>
####Explame
查看main.c文件

<br/>
####设计思想和实现
查看 http://www.94geek.com


