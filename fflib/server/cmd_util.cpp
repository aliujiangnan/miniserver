#include "server/cmd_util.h"
#include "server/ffworker.h"
using namespace ff;
using namespace std;

static void handleSessionCmd(SessionReqEvent& e){
    CmdHandlerPtr cmdHandler = CMD_MGR.getCmdHandler(e.cmd);
    if (cmdHandler){
        e.isDone = true;

        if (CMD_MGR.m_funcHook && CMD_MGR.m_funcHook(e.entity, e.cmd, e.data) == false){
            return;
        }
        cmdHandler->handleCmd(e.entity, e.cmd, e.data);
    }
}
bool CmdModule::init(){
    
    EVENT_BUS_LISTEN(&handleSessionCmd);
    return true;
}