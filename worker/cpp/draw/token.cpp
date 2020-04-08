#include "token.h"
using namespace draw;
    
TokenMgr* TokenMgr::instance = nullptr;
TokenMgr* TokenMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new TokenMgr();
    }
    return instance;
}
