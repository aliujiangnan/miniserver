
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <vector>
using namespace std;

#include "net/ioevent.h"
#include "net/acceptortcp.h"
#include "net/sockettcp.h"
#include "base/str_tool.h"
#include "net/socket_op.h"
#include "base/task_queue.h"
#include "base/log.h"
#include "net/socket_protocol.h"

using namespace ff;

AcceptorTcp::AcceptorTcp(IOEvent& e_, SocketProtocolFunc f):
    m_fdListen(-1),
    m_ioevent(e_),
    m_funcProtocol(f)
{
}

AcceptorTcp::~AcceptorTcp()
{
    this->close();
}

int AcceptorTcp::open(const string& address_)
{
    LOGTRACE(("ACCEPTOR", "accept begin %s", address_));
    //! example tcp://192.168.1.2:8080
    vector<string> vt;
    StrTool::split(address_, vt, "://");
    if(vt.size() != 2) return -1;

    vector<string> vt2;
    StrTool::split(vt[1], vt2, ":");
    if(vt2.size() != 2) return -1;

    //! 1.初始化一个addrinfo结构体
    struct addrinfo hints;
    bzero(&hints, sizeof(struct addrinfo) );
    hints.ai_family      = AF_UNSPEC;
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags        = AI_PASSIVE;

    int ret =0, yes = 1;
    struct addrinfo *res;

    const char* host = NULL;
    if (vt2[0] != "*") host = vt2[0].c_str();

    //! 2.获取地址信息写入一个addrinfo指针中
    if ((ret = getaddrinfo(host, vt2[1].c_str(), &hints, &res)) != 0) 
    {
        fprintf(stderr, "AcceptorTcp::open getaddrinfo: %s, address_=<%s>\n", gai_strerror(ret), address_.c_str());
        return -1;
    }

    Socketfd tmpfd = -1;
    Socketfd nInvalid = -1;

    //! 3.创建TCP socketfd
    if ((tmpfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == nInvalid)
    {
        perror("AcceptorTcp::open when socket");
        return -1;
    }

    //! 4.设置可重用本地地址与端口
    if (::setsockopt(tmpfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
    {
        perror("AcceptorTcp::open when setsockopt");
        return -1;
    }

    //! 5.绑定IP和端口
    if (::bind(tmpfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        fprintf(stderr, "AcceptorTcp::open when bind: %s, address_=<%s>\n", strerror(errno), address_.c_str());
        return -1;
    }

    //! 6.设置非阻塞模式
    SocketOp::set_nonblock(tmpfd);

    //! 7.设置最大等待连接数为5
    if (::listen(tmpfd, LISTEN_BACKLOG) == -1)
    {
        perror("AcceptorTcp::open when listen");
        return -1;
    }

    //! 8.释放addrinfo
    ::freeaddrinfo(res);
    m_fdListen = tmpfd;
    LOGTRACE(("ACCEPTOR", "accept ok %s", address_));
    return m_ioevent.regfdAccept(m_fdListen, funcbind(&AcceptorTcp::handleEvent, this));
}
void AcceptorTcp::handleEvent(Socketfd fdEvent, int eventType, const char* data, size_t len)
{
    if (eventType != IOEVENT_ACCEPT){
        return;
    }
    Socketfd newFd = *((Socketfd*)data);
    SocketProtocolPtr prot = new SocketProtocol(m_funcProtocol);
    SharedPtr<SocketTcp> skt = new SocketTcp(m_ioevent, funcbind(&SocketProtocol::handleSocketEvent, prot), newFd);
    skt->refSelf(skt);
    skt->open();
}
void AcceptorTcp::close()
{
    if (m_fdListen > 0)
    {
        m_ioevent.unregfd(m_fdListen);
        SocketOp::close(m_fdListen);
        m_fdListen = -1;
    }
}
