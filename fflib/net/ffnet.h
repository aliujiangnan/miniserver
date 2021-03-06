#ifndef _FF_NET_H_
#define _FF_NET_H_

#include "net/acceptortcp.h"
#include "net/ioevent_epoll.h"
#include "connector.h"
#include "base/singleton.h"
#include "base/perf_monitor.h"
#include "base/thread.h"
#include "net/msg_sender.h"
#include "base/log.h"

namespace ff {

class FFNet
{
public:
    volatile bool           flagStarted;
    TaskQueue*              tg;
    Thread                  thread;
    IOEventEpoll            ioevent;
    std::vector<Acceptor*>  allAcceptor;
public:
    static FFNet& instance(){
        return Singleton<FFNet>::instance();
    }
    FFNet():flagStarted(false), tg(NULL){
        start();
    }
    ~FFNet(){
        stop();
        for (size_t i = 0; i < allAcceptor.size(); ++i)
        {
            delete allAcceptor[i];
        }
        allAcceptor.clear();
        delete tg;
        tg = NULL;
    }
    static void runNet(void* e_)
    {
        FFNet* p = (FFNet*)e_;
        p->ioevent.run();
    }
    void start()
    {
        if (false == flagStarted)
        {
            flagStarted = true;
            tg = new TaskQueue();
            thread.create_thread(funcbind(&runNet, this), 1);
        }
    }
    void stop()
    {
        if (true == flagStarted)
        {
            for (size_t i = 0; i < allAcceptor.size(); ++i)
            {
                allAcceptor[i]->close();
            }

            tg->close();
            ioevent.stop();
            thread.join();

            flagStarted = false;
        }
    }
    Acceptor* listen(const std::string& host_, SocketProtocolFunc f)
    {
        AcceptorTcp* ret = new AcceptorTcp(ioevent, f);

        if (ret->open(host_))
        {
            delete ret;
            return NULL;
        }
        allAcceptor.push_back(ret);
        return ret;
    }
    SocketObjPtr connect(const std::string& host_, SocketProtocolFunc f)
    {
        return Connector::connect(host_, ioevent, f);
    }
};

}

#endif
