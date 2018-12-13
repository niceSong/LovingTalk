#include <iostream>
#include <cstdlib>
#include <mysql/mysql.h>


#include "DBctl.h"


DBctl::TDBctl::TDBctl()
{
	if((client = mysql_init(NULL)) == NULL)
	{
		std::cerr << "the database init failed" <<std::endl;
	}
}


DBctl::TDBctl::~TDBctl()
{
	if(resource)
	{
		mysql_free_result(resource);
	}
	if(client)
	{
		mysql_close(client);
	}
}


bool DBctl::TDBctl::ConnectDB(const std::string host, const std::string passwd, const std::string db, unsigned int port, unsigned int flag)
{
	char temp = 1;
	mysql_options(client, MYSQL_OPT_RECONNECT, &temp);    //set DB long connect 

	if(!mysql_real_connect(client, host.c_str(), NULL, passwd.c_str(), db.c_str(), port, NULL, flag))
	{
		std::cerr << "connect database error:" << mysql_error(client) << std::endl;	
		return false;
	}
	
	return true;
}

bool DBctl::TDBctl::InQuireDB(const std::string sql)   
{
	if(mysql_real_query(client, sql.c_str(), sql.size()))
	{
		std::cerr << "inQuire database error:" << mysql_error(client) << std::endl;
		return false;
	}
	if((resource = mysql_store_result(client)) == NULL)
	{
		std::cerr << "store database error:" << mysql_error(client) << std::endl;
		return false;
	}
	return true;
}

bool DBctl::TDBctl::ModifyDB(const std::string sql)
{
	if(mysql_real_query(client, sql.c_str(), sql.size()))
	{
		std::cerr << "inQuire database error:" << mysql_error(client) << std::endl;
		return false;
	}
	return true;
}

std::string DBctl::TDBctl::GetDBCode()
{
	std::string code;

	if(resource)
	{
		while((row = mysql_fetch_row(resource)) != NULL)
		{
			for(int j = 0; j < mysql_num_fields(resource); j++)
			{
				code = row[j];
				//std::cout << row[j] << '\t';
			}
			//std::cout << std::endl;
		}
		mysql_free_result(resource);
		resource = NULL;
	}

	return code;
}


