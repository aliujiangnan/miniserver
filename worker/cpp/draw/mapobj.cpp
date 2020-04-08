#include "mapobj.h"
using namespace draw;

MapObjMgr* MapObjMgr::instance = nullptr;
MapObjMgr* MapObjMgr::getInstance()
{
    if(instance == nullptr)
    {
        instance = new MapObjMgr();
    }
    return instance;
}
