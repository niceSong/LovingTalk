这是本人用来学习的项目，没有什么商业用途。
简单的外网可访问的高并发服务器.
服务对象是一款待开发的即时通信软件：LovingTalk。
目前这只是我的云备份，暂时无法使用，因为并发处理时还有BUG，以后再完善改进。
目前不支持任何协议栈，将来应支持sip和http协议栈。

这个项目包含的功能及对应源文件：
（1）socket网络编程，已通过花生壳实现了外网访问。（Connect.C）
（2）epoll高并发处理。(Main.C)
（3）多进程。(Main.C)
（4）最小堆算法实现的高效定时器。(Timer.C)
（5）心跳例检。（Timer.C）
（6）状态机的实现。(FSM.C)
（7）通过mysql数据库函数接口，访问mysql数据库。(DBctl.C)

编译：项目使用automake进行编译。
配置文件Makefile.am：
UTOMAKE_OPTIONS=foreign
bin_PROGRAMS = Loving_Talk
Loving_Talk_SOURCES = /home/tyy/Loving_Talk/src/Main.C /home/tyy/Loving_Talk/src/DBctl.C /home/tyy/Loving_Talk/src/FSM.C /home/tyy/Loving_Talk/src/Connect.C /home/tyy/Loving_Talk/src/Timer.C
Loving_Talk_CPPFLAGS = -I /home/tyy/Loving_Talk/include/
noinst_HEADERS = /home/tyy/Loving_Talk/include/Connect.h /home/tyy/Loving_Talk/include/DBctl.h /home/tyy/Loving_Talk/include/FSM.h /home/tyy/Loving_Talk/include/Timer.h
INCLUDES = -I/home/tyy/Loving_Talk/include/
LIBS= -L/usr/lib64/mysql -lmysqlclient -lpthread -lz
AM_CXXFLAGS=-std=gnu++11

--------------------------------------------不同语言描述（different language descriptions）------------------------------------------------

This is the project I used to study and there is no commercial use.
A simple high-end concurrent server accessible to the external network.
the service object is a real-time communication software to be developed: LovingTalk.
At present, this is just my cloud backup, which is temporarily unavailable, because there are still bugs in concurrent processing, and improvements will be made in the future.
Currently, no protocol stack is supported, and sip and http protocol stacks should be supported in the future.

The features and corresponding source files included in this project:
(1) socket network programming, has achieved access to the external network through the peanut shell. (Connect.C)
(2) epoll high concurrent processing. (Main.C)
(3) Multiple processes. (Main.C)
(4) Efficient timer implemented by the minimum heap algorithm. (Timer.C)
(5) Heartbeat routine examination. (Timer.C)
(6) Implementation of the state machine. (FSM.C)
(7) Access the mysql database through the mysql database function interface. (DBctl.C)

Compile: The project is compiled with automake.
Configuration file Makefile.am is:
UTOMAKE_OPTIONS=foreign
bin_PROGRAMS = Loving_Talk
Loving_Talk_SOURCES = /home/tyy/Loving_Talk/src/Main.C /home/tyy/Loving_Talk/src/DBctl.C /home/tyy/Loving_Talk/src/FSM.C /home/tyy/Loving_Talk/src/Connect.C /home/tyy/Loving_Talk/src/Timer.C
Loving_Talk_CPPFLAGS = -I /home/tyy/Loving_Talk/include/
noinst_HEADERS = /home/tyy/Loving_Talk/include/Connect.h /home/tyy/Loving_Talk/include/DBctl.h /home/tyy/Loving_Talk/include/FSM.h /home/tyy/Loving_Talk/include/Timer.h
INCLUDES = -I/home/tyy/Loving_Talk/include/
LIBS= -L/usr/lib64/mysql -lmysqlclient -lpthread -lz
AM_CXXFLAGS=-std=gnu++11
