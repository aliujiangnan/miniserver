
//!  连接器
#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "net/sockettcp.h"
#include "base/str_tool.h"
#include "net/socket_protocol.h"

namespace ff {

class Connector
{
public:
    static SocketObjPtr connect(const std::string& host_, IOEvent& e_, SocketProtocolFunc f)
    {
        SocketObjPtr ret = NULL;
        //! example tcp://192.168.1.1:1024
        std::vector<std::string> vt;
        StrTool::split(host_, vt, "://");
        if (vt.size() != 2) return NULL;

        std::vector<std::string> vt2;
        StrTool::split(vt[1], vt2, ":");
        if (vt2.size() != 2) return NULL;
        if (vt2[0] == "*")
        {
            vt2[0] = "127.0.0.1";
        }
        Socketfd s;
        struct sockaddr_in addr;

        //! 1.创建TCP socketfd
        if((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
        {
            perror("socket");
            return ret;
        }
        //! 2.初始化一个sockaddr_in结构体
        bzero(&addr,sizeof(addr));

        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(atoi(vt2[1].c_str())); //! 大端存储(网络字节顺序)
        addr.sin_addr.s_addr = inet_addr(vt2[0].c_str());
        
        //! 3.尝试建立连接
        if(::connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("connect");
            return ret;
        }
        SocketProtocolPtr prot = new SocketProtocol(f);
        SharedPtr<SocketTcp> skt = new SocketTcp(e_, funcbind(&SocketProtocol::handleSocketEvent, prot), s); //ref=1
        ret = skt;
        skt->refSelf(skt);                                                                                   //ref=2
        ret->open();
        return ret;                                                                                          //ref=1
    }

};

}
#endif
