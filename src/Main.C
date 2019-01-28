#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <sys/epoll.h>  


#include "FSM.h"
#include "Connect.h"
#include "Timer.h"

#define HBRECVMAX 4
#define NETRECVMAX 10


//心跳socket
NSLink::T_Link THBServerLink(AF_INET, SOCK_STREAM, 0);


int main (int argc, char *argv[])
{
	int testepoll = 0;	

	int HB_epollfd = 0;
	
	int HBReadyFdNum = 0;
	int NETReadyFdNum = 0;
	int EVENTFdNum = 0;
	int tmpnum = 0;     //循环访问epoll事件
	
	int pid;       //存放子进程pid
	int fork_pid;  //存放fork后产生值
	int state = 1;  //等于0，触发test消息。
	int ResetTimerFlag = 0;  //为1表示定时器已重置
	
	char Precvbuf[HBRECVMAX];   //存放心跳消息
	memset(Precvbuf, 0, HBRECVMAX);
	
	char PNetbuf[NETRECVMAX];    //存放网络信息


	/*父进程epoll相关结构定义*/	
	struct epoll_event ev;            //心跳epoll
	struct epoll_event events[2048];      //心跳epoll


	/* 存放键值对：pid、T_Link类指针 */
	std::unordered_map<int, NSLink::T_Link *> THBClientLinkMap;
	std::unordered_map<int, NSLink::T_Link *> TNETClientLinkMap;
	//std::unordered_map<int, NSLink::T_Link *>::iterator mapit;
	
	/* 子进程中心跳使用的socket */
	NSLink::T_Link *THBClientLink1 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink2 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink3 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *THBClientLink4 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);

	/* 子进程中网络使用的socket */
	NSLink::T_Link *TNETClientLink1 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *TNETClientLink2 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *TNETClientLink3 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);
	NSLink::T_Link *TNETClientLink4 = new NSLink::T_Link(AF_INET, SOCK_STREAM, 0);

	/* 心跳socket的vector */
	std::vector<NSLink::T_Link *> THBClientLinkVec = { THBClientLink1, THBClientLink2, THBClientLink3, THBClientLink4 };
	auto HBvecit = THBClientLinkVec.begin();

	/* 网络socket的vector */
	std::vector<NSLink::T_Link *> TNETClientLinkVec = { TNETClientLink1, TNETClientLink2, TNETClientLink3, TNETClientLink4 };
	auto NETvecit = TNETClientLinkVec.begin();

	T_HeapCtl THBtimerHeap(10);


	/*父进程接受心跳socekt*/
	THBServerLink.LTHBsetsockopt();
	//std::cout << "HBserver,socket_fd:" << THBServerLink.socket_fd <<std::endl;
	THBServerLink.LTbind(HB_PORT_SERVER);
	THBServerLink.LTSetnonblock(THBServerLink.socket_fd);  //设置非阻塞
	
	HB_epollfd = epoll_create(30);
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = THBServerLink.socket_fd;
	/* 添加心跳的socket_fd */
	if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.socket_fd, &ev) < 0 )   //加入心跳socket_fd
	{
		close(THBServerLink.socket_fd);
		close(HB_epollfd);
		std::cout << "epoll_ctl failed!" <<std::endl;
		exit(1);
	}
	THBServerLink.LTlisten();




	/*
	for(int i = 0; i < 4; i++)
	{
		netev.events = EPOLLIN;
		netev.data.fd = (*NETvecit)->socket_fd;
		std::cout << "TNETClientLink->socket_fd : " << (*NETvecit)->socket_fd << std::endl;
		(*NETvecit)->LTshowRW((*NETvecit)->socket_fd);
		if(epoll_ctl(NET_epollfd, EPOLL_CTL_ADD, (*NETvecit)->socket_fd, &netev) < 0 )
		{
			close((*NETvecit)->socket_fd);
			close(NET_epollfd);
			std::cout << "epoll_ctl failed!" <<std::endl;
			exit(1);
		}
		NETvecit++;
	}*/
	NETvecit = TNETClientLinkVec.begin();

	
	/* 网络 acceptfd epoll */
	/*
	int EVENT_epollfd = 0;
	struct epoll_event EVENTev; 	  //事件处理epoll
	struct epoll_event EVENTevents[2048];//事件处理epoll
	EVENT_epollfd = epoll_create(30);
	memset(&EVENTev, 0, sizeof(EVENTev));
	*/



	//创建4个子进程
	HBvecit = THBClientLinkVec.begin();
	for(int i = 0; i < 4; i++)   
	{		
		fork_pid = fork();
		if(0 == fork_pid)
		{
			int tmppid = (int)getpid();
			/* 添加心跳socket到Map */
			THBClientLinkMap[tmppid] = *HBvecit;
			/* 添加网络socket到Map */
			TNETClientLinkMap[tmppid] = *NETvecit;
			
			/* 打印测试网络socket是否成功添加进Map */
			auto mapit = TNETClientLinkMap.find(tmppid);
			std::cout << "mapit found" << mapit->first << "ls" << mapit->second << std::endl;
		}
		HBvecit++;     //该值以改变，今后使用vecit前记得初始化一下。
		NETvecit++;    
		
		if(fork_pid == 0 || fork_pid < 0)    //若是子进程就会退出，防止子进程再次创建子进程
			break;
	}
	if(fork_pid < 0)
	{
		std::cout << "fork error!" << std::endl;
		exit(1);
	}


	/*******************************************************************************************/
	/************************* 子进程循环：发心跳、处理客户端发来消息 **************************/
	/*******************************************************************************************/
	if(0 == fork_pid) 
	{	
		//获得进程id
		pid = (int)getpid();
		//创建定时器对象
		TSendHBTimer TsendHBtimer(pid);	

		
		
		/* 创建子进程心跳socket */		
		NSLink::T_Link *THBClientLink = nullptr;
		auto HBmapit = THBClientLinkMap.find(pid);
		if(HBmapit != THBClientLinkMap.end())
		{
			THBClientLink = HBmapit->second;
		}
		
		std::cout << "THBClientLink->socket_fd : " << THBClientLink->socket_fd << std::endl;
		THBClientLink->LTHBsetsockopt();

		
		THBClientLink->LTConnect("192.168.1.111", HB_PORT_SERVER);   //发起三次握手
		THBClientLink->LTsend((char *)&pid, sizeof(int), 0);   //发心跳
		std::cout << "in LTsend " << std::endl;


		/* 创建子进程网络socket */
		NSLink::T_Link TNETClientLink(AF_INET, SOCK_STREAM, 0);
		TNETClientLink.LTNETsetsockopt();
		TNETClientLink.LTbind(SREVER_PORT);		
		//TNETClientLink.LTSetnonblock(TNETClientLink.socket_fd);
		TNETClientLink.LTlisten();
		/*
		NSLink::T_Link *TNETClientLink = nullptr;
		auto NETmapit = TNETClientLinkMap.find(pid);
		if(NETmapit != TNETClientLinkMap.end())
		{
			TNETClientLink = NETmapit->second;
		}		
		TNETClientLink->LTNETsetsockopt();
		TNETClientLink->LTbind(SREVER_PORT);		
		TNETClientLink->LTSetnonblock(TNETClientLink->socket_fd);
		TNETClientLink->LTlisten();
		*/

		/* 网络 socketfd epoll，添加socket_fd */
		int NET_epollfd = 0;
		struct epoll_event netev;         //网络消息epoll
		struct epoll_event netevents[2048];   //网络消息epoll
		NET_epollfd = epoll_create(30);
		memset(&netev, 0, sizeof(netev));
		netev.events = EPOLLIN | EPOLLET;
		netev.data.fd = TNETClientLink.socket_fd;
		std::cout << "TNETClientLink.socket_fd : " << TNETClientLink.socket_fd << std::endl;
		TNETClientLink.LTshowRW(TNETClientLink.socket_fd);
		if(epoll_ctl(NET_epollfd, EPOLL_CTL_ADD, TNETClientLink.socket_fd, &netev) < 0 )
		{
			close(TNETClientLink.socket_fd);
			close(NET_epollfd);
			std::cout << "epoll_ctl failed!" <<std::endl;
			exit(1);
		}

		//状态机
		TFSM Tfsm;
		TState_Test *Tstate_test = new TState_Test();
		Tfsm.Register(Test, Tstate_test);

		
		while(1)
		{		
			/* 定时发心跳 */
			TsendHBtimer.TimeOutFunc(THBClientLink);

			/* 监听网络epoll */
			NETReadyFdNum = epoll_wait(NET_epollfd, netevents, 20, 1);
			
			/* socket_fd变化 */
			if(NETReadyFdNum < 0)
			{
				std::cout << "child fork epoll_wait error!" <<std::endl;
				break;
			}				
			if(NETReadyFdNum > 0)
			{
				std::cout << "NETReadyFdNum is : " << NETReadyFdNum << std::endl;
				for(int i = 0; i < NETReadyFdNum; i++)
				{
					if(netevents[i].data.fd == TNETClientLink.socket_fd)
					{
						std::cout << "in NETReadyFdNum,socket_fd is: " << TNETClientLink.socket_fd << std::endl;
						TNETClientLink.LTshowRW(TNETClientLink.socket_fd);
						//accept
						TNETClientLink.LTaccept();
						if(TNETClientLink.accept_fd >= 0)
						{
							//epoll
							netev.events = EPOLLIN | EPOLLET;
							netev.data.fd = TNETClientLink.accept_fd;
							std::cout << "NET accept_fd is : " << TNETClientLink.accept_fd << std::endl;
							if(epoll_ctl(NET_epollfd, EPOLL_CTL_ADD, TNETClientLink.accept_fd, &netev) < 0 )   //加入心跳socket_fd
							{
								close(TNETClientLink.accept_fd);
								close(NET_epollfd);
								std::cout << "Deal event epoll_ctl failed!" <<std::endl;
								exit(1);
							}
						}
					}
					else
					{
						//接受数据
						std::cout << "i is : " << i <<std::endl;
						std::cout << "pid is : " << (int)getpid() <<std::endl;
						std::cout << "netevents accept_fd is : " << netevents[i].data.fd << std::endl;
						TNETClientLink.LTrecv(netevents[i].data.fd, PNetbuf, NETRECVMAX, 0);
						memcpy(&state, PNetbuf, sizeof(int));
						
						Tfsm.GetStateRun(state);
					}

				}
			}

		}
		std::cout << "out of child while(1) " <<std::endl;
	}


	/******************************************************************************************/
	/********************** 父进程循环：创建子进程、收心跳、收客户端消息 **********************/
	/******************************************************************************************/
	else 
	{
		
		while(1)
		{
			HBReadyFdNum = epoll_wait(HB_epollfd, events, 20, 1);  //心跳(socket_fd、accept_fd)，服务端(socket_fd、accept_fd)
			if(HBReadyFdNum < 0)
			{
				std::cout << "father fork epoll_wait error!" <<std::endl;
				std::cout << strerror(errno) << std::endl;
				break;
			}
			if(HBReadyFdNum > 0)
			{
				for(tmpnum = 0; tmpnum < HBReadyFdNum; tmpnum++)
				{
					/* THBServerLink.socket_fd变化，调用accept */
					if(events[tmpnum].data.fd == THBServerLink.socket_fd) 
					{
						std::cout << "events.data.fd(socket_fd) is" << events[0].data.fd <<std::endl;
						THBServerLink.LTaccept();
						/* epoll添加心跳accept_fd */
						ev.events = EPOLLIN;
						ev.data.fd = THBServerLink.accept_fd;
						std::cout << "HB accept_fd is :" << THBServerLink.accept_fd <<std::endl;
						if(epoll_ctl(HB_epollfd, EPOLL_CTL_ADD, THBServerLink.accept_fd, &ev) < 0 )   //epoll监听socket_fd
						{
							close(THBServerLink.accept_fd);
							close(THBServerLink.socket_fd);
							close(HB_epollfd);
							std::cout << "epoll_ctl failed!" <<std::endl;
							exit(1);
						}
					}
					else   //THBServerLink.accept_fd变化，(1)创建定时器(2)重置定时器。
					{
						/* 接收到消息 */
						std::cout << "events.data.fd(accept_fd) is :" << events[tmpnum].data.fd <<std::endl;
						THBServerLink.LTrecv(events[tmpnum].data.fd, Precvbuf, sizeof(int), 0);
						memcpy(&pid, Precvbuf, sizeof(int));
						//pid = (int)(*Precvbuf);
						std::cout << "pid is:" << pid << std::endl;
						
						if(-1 == THBtimerHeap.cur_HeapSize)  //定时器heap为空
						{
							std::cout << "add timer in empty heap" << std::endl;
							TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
							THBtimerHeap.AddTimer(TrecvHBtimer);

						}  
						else   //定时器heap不为空
						{
							for(int i = 0; i <= THBtimerHeap.cur_HeapSize; i++)
							{
								T_Timer **tmp = THBtimerHeap.ReturnHeap();
								if(pid == tmp[i]->Pid)  //定时器已存在
								{
									//重置定时器
									std::cout << "reset timer" << std::endl;
									THBtimerHeap.ServerResetTimer(pid, 10);
									ResetTimerFlag = 1;
									break;
								}

							}
							if(0 == ResetTimerFlag)
							{
								//创建定时器
								std::cout << "add timer" << std::endl;
								TrecvHBTimer *TrecvHBtimer = new TrecvHBTimer(pid);
								THBtimerHeap.AddTimer(TrecvHBtimer);
								ResetTimerFlag = 0;
							}
						}
						
					}
				}
			}
			//检查定时器超时。这个THBClientLink1 指针实际上并不会用到
			THBtimerHeap.DealTimeOut(THBClientLink1); 
		}
		exit(1);  //跳出循环即不正常
	}


}



