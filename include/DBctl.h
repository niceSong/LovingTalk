#ifndef DBctl_H_
#define DBctl_H_


namespace DBctl
{
	class TDBctl {
	public :
		
		TDBctl();	//init DB
		
		~TDBctl();	
		
		//connect DB
		bool ConnectDB(std::string host, std::string passwd, std::string db, unsigned int port, unsigned int flag);
		bool InQuireDB(std::string sql); 
		bool ModifyDB(std::string sql); 
		std::string GetDBCode(); //get information every row from DB
	 
	private:
		MYSQL *client = NULL;
		MYSQL_RES *resource = NULL;
		MYSQL_ROW row;
	};   //TDBctl

}//DBctl


#endif    //DBctl_H_