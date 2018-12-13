#ifndef CONNECT_H_
#define CONNECT_H_


#define SREVER_PORT 33333     //����ͻ��ˣ����ʶ˿�
#define HB_PORT_CLIENT 33334  //�����ͻ��ˣ����Ͷ˿�
#define HB_PORT_SERVER 33335  //���������������ն˿�



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
		void LTsetsockopt();            //���ö���̿��Թ���ĳһ�˿�
		void LTSetnonblock(int fd);		   //����fdΪ������
		
		int 	   socket_fd;      //bind�󲻻�ı���
		int 	   accept_fd;      //��ÿ�������ڲ��ϵı�ã��ú�ǵ��ͷ�

	private:
		const int  flag;
		
	};    //class T_Link

}    //NSLink

//int NSLink::T_Link::flag = 1; 




#endif    //CONNECT_H_
