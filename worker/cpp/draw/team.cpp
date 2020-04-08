#include "team.h"
using namespace draw;

TeamMgr* TeamMgr::instance = nullptr;
TeamMgr* TeamMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new TeamMgr();
    }
    return instance;
}
