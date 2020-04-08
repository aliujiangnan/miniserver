#include <stdio.h>
#include "base/fftype.h"
#include "base/arg_helper.h"
#include "base/str_tool.h"
#include "base/smart_ptr.h"
#include "base/log.h"
#include "base/signal_helper.h"
#include "base/perf_monitor.h"
#include "base/event_bus.h"
#include "base/os_tool.h"

#include "net/ffnet.h"
#include "rpc/ffrpc.h"
#include "server/ffworker.h"
#include "server/ffgate.h"
#include "server/http_mgr.h"
#include "server/db_mgr.h"

#include "./ffworker_python.h"

using namespace ff;
using namespace std;

int main(int argc, char* argv[])
{
    SignalHelper::bloack();
    ArgHelper& arg_helper = Singleton<ArgHelper>::instance();
	arg_helper.load(argc, argv);
	std::string cfgfile = "mini.conf";
    if (arg_helper.isEnableOption("-f"))
    {
        cfgfile = arg_helper.getOptionValue("-f");
    }
    else if (OSTool::isFile("../../mini.conf")){
        cfgfile = "../../mini.conf";
    }
    arg_helper.loadFromFile(cfgfile);

    //! 美丽的日志组件，shell输出是彩色滴！！
    if (arg_helper.isEnableOption("-log_path"))
    {
        LOG.start(arg_helper);
    }
    else
    {
        LOG.start("-log_path ./log -log_filename log -log_class DB_MGR,GAME_LOG,BROKER,FFRPC,FFGATE,FFWORKER,FFWORKER_PYTHON,FFWORKER_LUA,FFWORKER_JS,FFNET,HHTP_MGR -log_print_screen true -log_print_file true -log_level 4");
    }
    string perf_path = "./perf";
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

    try
    {
        int worker_index = 0;
        if (arg_helper.isEnableOption("-worker_index"))
        {
            worker_index = ::atoi(arg_helper.getOptionValue("-worker_index").c_str());
        }

        Singleton<HttpMgr>::instance().start();

        std::string brokercfg = "tcp://127.0.0.1:43210";
        if (arg_helper.isEnableOption("-broker")){
            brokercfg = arg_helper.getOptionValue("-broker");
        }

        std::string entry = "worker/py/mini/main.py";
        if (arg_helper.isEnableOption("-entry")){
            entry = arg_helper.getOptionValue("-entry");
            printf("use entry %s\n", entry.c_str());
        }
        else{
            if (OSTool::isFile("mini/main.py")){
                entry = "mini/main.py";
            }
            printf("use default entry %s, user -entry redirect entry script\n", entry.c_str());
        }

        if (Singleton<FFWorkerPython>::instance().open(brokercfg, worker_index))
        {
            printf("FFWorkerPython open error!\n");
            goto err_proc;
        }
        if (Singleton<FFWorkerPython>::instance().workerInit(entry))
        {
            printf("FFWorkerPython workerInit error!\n");
            goto err_proc;
        }
    }
    catch(exception& e_)
    {
        printf("exception=%s\n", e_.what());
        return -1;
    }

    SignalHelper::wait();

err_proc:
    Singleton<FFWorkerPython>::instance().close();
    PERF_MONITOR.stop();
    usleep(100);
    FFNet::instance().stop();
    usleep(200);

    Singleton<HttpMgr>::instance().stop();

    return 0;
}

