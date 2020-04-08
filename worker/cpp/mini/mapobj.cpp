#include "mapobj.h"
using namespace mini;

MapObjMgr* MapObjMgr::instance = nullptr;
MapObjMgr* MapObjMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new MapObjMgr();
    }

    return instance;
}
