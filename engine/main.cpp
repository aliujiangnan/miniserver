#include <stdio.h>

#include "base/fftype.h"
#include "base/arg_helper.h"
#include "base/str_tool.h"
#include "base/smart_ptr.h"
#include "base/log.h"
#include "base/signal_helper.h"
#include "base/perf_monitor.h"
#include "rpc/ffrpc.h"
#include "rpc/ffbroker.h"
#include "server/ffgate.h"
#include "net/ffnet.h"
#include "base/func.h"

using namespace ff;
using namespace std;

int main(int argc, char* argv[])
{
	ArgHelper arg_helper(argc, argv);

    if (arg_helper.isEnableOption("-f"))
    {
        arg_helper.loadFromFile(arg_helper.getOptionValue("-f"));
    }

    SignalHelper::bloack();

    //! 美丽的日志组件，shell输出是彩色滴！！
    if (arg_helper.isEnableOption("-log_path"))
    {
        LOG.start(arg_helper);
    }
    else
    {
        LOG.start("-log_print_screen true -log_level 6");
    }

    std::string perf_path = "./perf";
    long perf_timeout = 10*60;//! second
    if (arg_helper.isEnableOption("-perf_path"))
    {
        perf_path = arg_helper.getOptionValue("-perf_path");
    }
    if (arg_helper.isEnableOption("-perf_timeout"))
    {
        perf_timeout = ::atoi(arg_helper.getOptionValue("-perf_timeout").c_str());
    }
    if (PERF_MONITOR.start(perf_path, perf_timeout))
    {
        return -1;
    }
    
    FFGate ffgate;
    try
    {
        string gate_listen = "tcp://*:44000";
        //! 启动broker，负责网络相关的操作，如消息转发，节点注册，重连等
        if (arg_helper.isEnableOption("-gate_listen"))
        {
            gate_listen = arg_helper.getOptionValue("-gate_listen");
        }
        std::string brokercfg = "tcp://127.0.0.1:43210";
        if (FFBroker::instance().open(brokercfg))
        {
            printf("broker open failed\n");
            goto err_proc;
        }

        if (ffgate.open(brokercfg, gate_listen))
        {
            printf("gate open error!\n");
            goto err_proc;
        }
        printf("gate open ok\n");
    }
    catch(exception& e_)
    {
        printf("exception=%s\n", e_.what());
        return -1;
    }

    SignalHelper::wait();

err_proc:
    ffgate.close();
    PERF_MONITOR.stop();
    usleep(100);
    FFNet::instance().stop();
    usleep(200);

    return 0;
}
