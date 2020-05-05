//脚本

#include "./ffworker_cpp.h"
#include "base/perf_monitor.h"
#include "server/http_mgr.h"

#define _DRAW 1
#ifdef _DRAW
#include "draw/main.h"
using namespace draw;
#else
#include "mini/main.h"
using namespace mini;
#endif
using namespace ff;
using namespace std;

FFWorkerCpp::FFWorkerCpp():m_started(false)
{
}
FFWorkerCpp::~FFWorkerCpp()
{
}
int FFWorkerCpp::workerInit()
{
    
    ArgHelper& arg_helper = Singleton<ArgHelper>::instance();
    if (arg_helper.isEnableOption("-db")){
        if (DbMgr::instance().initDBPool(arg_helper.getOptionValue("-db"), 1)){
            LOGERROR((FFWORKER_CPP, "FFWorker::db connect failed"));
            return -1;
        }
    }

    int ret = -2;
    
    try{
        
        Mutex                    mutex;
        ConditionVar            cond(mutex);
        
        getRpc().getTaskQueue().post(funcbind(&FFWorkerCpp::processInit, this, &mutex, &cond, &ret));
        LOGINFO((FFWORKER_CPP, "FFWorkerCpp::begin init"));
        LockGuard lock(mutex);
        if (ret == -2){
            cond.wait();
        }
        LOGINFO((FFWORKER_CPP, "FFWorkerCpp::processInit return"));
        if (ret < 0)
        {
            this->close();
            return -1;
        }
    }
    catch(exception& e_)
    {
        return -1;
    }
    m_started = true;
    GameMain::init();
    LOGTRACE((FFWORKER_CPP, "FFWorkerCpp::workerInit end ok"));
    return ret;
}

//!!处理初始化逻辑
int FFWorkerCpp::processInit(Mutex* mutex, ConditionVar* var, int* ret)
{
    try{
        if (this->initModule()){
            *ret = 0;
        }
        else
            *ret = -1;
    }
    catch(exception& e_)
    {
        *ret = -1;
        LOGERROR((FFWORKER_CPP, "FFWorkerCpp::open failed er=<%s>", e_.what()));
    }
    LockGuard lock(*mutex);
    var->signal();
    LOGINFO((FFWORKER_CPP, "FFWorkerCpp::processInit end"));
    return 0;
}
int FFWorkerCpp::close()
{
    FFWorker::close();
    if (false == m_started)
        return 0;
    m_started = false;

    LOGINFO((FFWORKER_CPP, "FFWorkerCpp::close end"));
    GameMain::cleanUp();
    return 0;
}
int FFWorkerCpp::onSessionReq(userid_t session_id, uint16_t cmd, const std::string& data)
{
    GameMain::onSessionReq(session_id, cmd, data);
    return 0;
}

int FFWorkerCpp::onSessionOffline(userid_t session_id)
{
    GameMain::onSessionOffline(session_id);
    return 0;
}
int FFWorkerCpp::onSessionEnter(userid_t session_id, const std::string& extra_data)
{
    return 0;
}

string FFWorkerCpp::onWorkerCall(uint16_t cmd, const std::string& body)
{
    return "";
}
