简单高并发服务器：   
无商用。   
大致功能：触发epoll读事件，由某一个进程读取socket数据，并由该进程完成数据触发的相关操作，比如：打印测试字符串、访问数据库等。
相关技术：socket网络编程、多进程、epoll高并发惊群处理、访问数据库、最小堆定时器、状态机、心跳例检。   
automake方式编译。   
