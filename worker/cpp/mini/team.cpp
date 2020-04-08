#include "team.h"
using namespace mini;

TeamMgr* TeamMgr::instance = nullptr;
TeamMgr* TeamMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new TeamMgr();
    }
    return instance;
}
