#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdint.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>

namespace ff {

struct MessageHead
{
    MessageHead():
        size(0),
        cmd(0),
        flag(0)
    {}
    explicit MessageHead(uint16_t cmd_):
        size(0),
        cmd(cmd_),
        flag(0)
    {}
    void hton()
    {
        printf("size1,%d\n",size);
        size = htonl(size);
        printf("size2,%d\n",size);
        cmd  = htons(cmd);
        flag = htons(flag);
    }
    void ntoh()
    {
        printf("size3,%d\n",size);
        size = ntohl(size);
        printf("size4,%d\n",size);
        cmd  = ntohs(cmd);
        flag = ntohs(flag);
    }
    uint32_t size;
    uint16_t cmd;
    uint16_t flag;
};

class Message
{
public:
    Message()
    {
    }

    uint16_t getCmd() const               { return m_head.cmd; }
    const std::string& getBody() const         { return m_body;}
    size_t size() const                    { return m_head.size; }
    uint16_t getFlag() const              { return m_head.flag; }

    size_t appendHead(size_t index_, const char* buff, size_t len)
    {
        printf("size5,%d\n",m_head.size);
        size_t will_append = sizeof(m_head) - index_;
        if (will_append > len)
        {
            will_append = len;
            ::memcpy((char*)&m_head + index_, buff, will_append);
        }
        else
        {
            ::memcpy((char*)&m_head + index_, buff, will_append);
            m_head.ntoh();
        }
        printf("size7,%d\n",m_head.size);
        return will_append;
    }
    size_t appendMsg(const char* buff, size_t len)
    {
        printf("size6,%d\n",m_head.size);
        size_t will_append = m_head.size - m_body.size();
        if (will_append > len) will_append = len;
        m_body.append(buff, will_append);
        printf("size8,%d\n",m_head.size);
        return will_append;
    }
    void clear()
    {
        ::memset(&m_head, 0, sizeof(m_head));
        m_body.clear();
    }
    void appendToBody(const char* buff, size_t len)
    {
        m_body.append(buff, len);
    }
    //! for parse
    bool haveRecvHead(size_t have_recv_size_) { return have_recv_size_ >= sizeof(MessageHead);}
    
    MessageHead& getHead() { return m_head; }
private:
    MessageHead m_head;
    std::string         m_body;
};
    
}
#endif
