## 项目主要包含四部分
~~现在还在开发~~
- server 
- client
- mq
- worker

## 文件分布
### server模块
在src文件里面，包括了线程池，线程池数据对象，server的源码，带一个MQ的publisher
### client 模块
在client文件夹里，现在还比较简陋，暂时只用来测试连接。
### MQ模块
MQ/ 写废了的，内置了一个定时器，还把this指针传到信号处理函数中，跑起来有问题，问题还没排查所以没用上。   
src/ipc/ 可以跑起来的，封装了三种对象，一些些功能还在完善。consumer_test exchanger publisher_test是测试文件，exchanger可以直接用来服务在server和worker之间  
### worker模块
一个沙盒，沙盒中有一个MQ的consumer对象负责拉取消息。

### 其他
带test关键词的基本都是测试文件。  
代码写了很多注释。。。各种脚本都很简单为了方便编译调试 ，check是验证测试的正确性