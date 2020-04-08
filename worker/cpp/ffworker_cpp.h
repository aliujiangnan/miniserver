#ifndef _FF_FFWORKER_CPP_H_
#define _FF_FFWORKER_CPP_H_

#include "base/log.h"
#include "server/db_mgr.h"
#include "server/fftask_processor.h"
#include "server/ffworker.h"

namespace ff
{
#define FFWORKER_CPP "FFWORKER_CPP"

class FFWorkerCpp: public FFWorker, task_processor_i
{
public:
    FFWorkerCpp();
    ~FFWorkerCpp();

    int                     close();
    int                     processInit(Mutex* mutex, ConditionVar* var, int* ret);
    int                     workerInit();

    //! 转发client消息
    virtual int onSessionReq(userid_t session_id_, uint16_t cmd_, const std::string& data_);
    //! 处理client 下线
    virtual int onSessionOffline(userid_t session_id);
    //! 处理client 跳转
    virtual int onSessionEnter(userid_t session_id, const std::string& extra_data);
    //! scene 之间的互调用
    virtual std::string onWorkerCall(uint16_t cmd, const std::string& body);
    
public:
    bool                    m_started;
};

}

#endif