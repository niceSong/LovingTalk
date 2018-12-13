#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "FSM.h"


TFSM::~TFSM()
{
	std::unordered_map<int, TState *>::iterator iter;
	for(iter = all_states.begin(); iter != all_states.end(); iter++)
	{
		delete iter->second;
	}
}


void TFSM::Register(int state, TState *Tstate)
{
	std::unordered_map<int, TState *>::iterator iter = all_states.find(state);
	if (iter == all_states.end())
		std::cout << "Current don't develop this state" << std::endl;
	else
	{
		iter->second = Tstate;
	}
}


void TFSM::GetStateRun(int state)
{
	std::unordered_map<int, TState *>::iterator iter;
	for(iter = all_states.begin(); iter != all_states.end(); iter++)
	{
		if(state == iter->first)
		{
			iter->second->Run();
			break;
		}
	}
	if (iter == all_states.end())
		std::cout << "can't get state run function" << std::endl;
}


void TState_Test::Run()
{
	//打印表示已接收到数据
	std::cout << "success:recv test message!" << std::endl;
	
}



