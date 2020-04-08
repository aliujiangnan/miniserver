#ifndef _DRAW_TOKEN_H_
#define _DRAW_TOKEN_H_

#include <sys/time.h>
#include <string>
#include <map>

namespace draw
{
struct Token
{
    Token(const int& _id, const int& _time, const int& _period):
    id(_id),
    time(_time),
    period(_period){}

    int id;
    int time;
    int period;
};

struct TokenMgr
{
    TokenMgr(){}
    
	static TokenMgr* getInstance();

    std::string newToken(const int& id, const int& period) 
	{
		const std::string& token = allUsers[id];
		if (token != "") 
		{
			delToken(token);
		}

		timeval tv;
		gettimeofday(&tv, NULL);

		std::string tokenStr = sign(std::to_string(id) + "!@#$%^&" + std::to_string(tv.tv_usec));
		allTokens[tokenStr] = new Token(id, tv.tv_usec, period);
		allUsers[id] = token;
		return token;
	}

	std::string getToken(const int& id)
	{
		return allUsers[id];
	}

	int getUserID(const std::string& token) 
	{
		return allTokens[token]->id;
	}

	bool isValid(const std::string& token) 
	{
		Token* info = allTokens[token];
		if (info) {
			return false;
		}

		timeval tv;
		gettimeofday(&tv, NULL);
		if (info->time + info->period < tv.tv_usec) 
		{
			return false;
		}
		return true;
	}

	void delToken(const std::string& token)
	{
		Token* info = allTokens[token];
		if (info) 
		{
			allTokens.erase(token);
			delete info;
			allUsers.erase(info->id);
		}
	}

	std::string sign(const std::string& str)  
	{
		return "";
	}

    std::map<std::string, Token*> allTokens;
    std::map<int, std::string> allUsers;
	static TokenMgr* instance;
};
}
#endif
