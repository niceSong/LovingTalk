# LovingTalk
简单server，用来学习高并发服务器。      
## 大致功能
接受client端消息，触发epoll读事件，由某一个进程读取socket数据，并由该进程处理业务逻辑。    
## 相关技术
* socket网络编程
* fork
* epoll高并发惊群处理
* 数据库mysql
* 最小堆定时器
* 状态机   
## 编译
automake。   
