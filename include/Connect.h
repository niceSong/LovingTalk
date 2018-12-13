#ifndef CONNECT_H_
#define CONNECT_H_


#define SREVER_PORT 33333     //网络客户端，访问端口
#define HB_PORT_CLIENT 33334  //心跳客户端，发送端口
#define HB_PORT_SERVER 33335  //心跳服务器，接收端口



namespace NSLink
{
	class T_Link
	{
	public:
		T_Link(int domain, int type, int protocol);   //socket's package
		~T_Link();
		void LTbind(unsigned short port);
		void LTlisten();
		void LTConnect(const char* ip,int port);
		void LTaccept();
		void LTsend(const char   *buf, int len, int flags );
		void LTrecv(int fd, char  *buf, int len, int flags );
		void LTsetsockopt();            //设置多进程可以共享某一端口
		void LTSetnonblock(int fd);		   //设置fd为非阻塞
		
		int 	   socket_fd;      //bind后不会改变了
		int 	   accept_fd;      //在每次连接在不断的变幻，用后记得释放

	private:
		const int  flag;
		
	};    //class T_Link

}    //NSLink

//int NSLink::T_Link::flag = 1; 




#endif    //CONNECT_H_
