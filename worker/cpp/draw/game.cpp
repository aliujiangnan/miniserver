#include "game.h"
using namespace draw;

GameMgr* GameMgr::instance = nullptr;
GameMgr* GameMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new GameMgr();
    }
    return instance;
}
