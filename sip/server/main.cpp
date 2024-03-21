#include <iostream>
#ifdef _WIN32_WCE
/* #include <syslog.h> */
#include <winsock2.h>
#endif
#include "csafe_queue.h"
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include <cstdint>
#include <thread>
#include <functional>

eXosip_t* g_excontext_ptr = NULL;

typedef std::function<int(const sip_event_t&)> event_proc;
struct sip_event_t
{
    int                 value;
    const char* name;
    event_proc          proc;
    struct eXosip_t* excontext;
    eXosip_event_t* exevent;
    uint64_t            id;

    sip_event_t() = default;
};

chen::csafe_queue< sip_event_t* > g_sip_event_queue;



/*

local_sip:
  svr_id: 37010200002000000001
  svr_domain: 3701030000
  svr_ip: 192.168.81.141
  svr_port: 5566

remote_sip:
  svr_id: 34020000002000000001
  svr_domain: 3402000000
  svr_ip: 192.168.81.141
  svr_port: 5060
  passwd: 123456

log:
  enable: off
  level: 1
  path: /log/gb28181/
  logName: gb28181.log

#媒体接收者IP
media_raddr:
  ip: 172.18.3.203

media_port_pool:
  start: 50000
  end: 54000
*/


std::thread g_recv_thread;
std::thread g_send_thread;



int sip_recv_thread_event()
{
    eXosip_event_t* exosip_event;

    exosip_event = eXosip_event_wait(g_excontext_ptr, 0, 1);

    //    eXosip_lock(m_excontext);
    //    eXosip_automatic_action(m_excontext);
    //    eXosip_unlock(m_excontext);

    if (exosip_event == nullptr)
        return 0;

    sip_event_sptr sipg_event = new_event(g_excontext_ptr, exosip_event);
    if (nullptr == sipg_event)
        return 0;

    m_eventQueue.Push(sipg_event);
    LOG_DEBUG << "Push event: " << sipg_event->name << ", id=" << sipg_event->id << " to queue successfully";

}

int main(int argc, char* argv[])
{
    const char* local_svr_id = "37010200002000000001";
    const char* local_ip = "192.168.0.191";
    const uint32_t local_port = 5566;
	g_excontext_ptr = eXosip_malloc();

    int ret = eXosip_init(g_excontext_ptr);
    if (ret != 0) 
    {
        printf("eXosip_init failed, ret= = %u\n", ret);
        //LOG_INFO << "eXosip_init failed, ret=" << ret;
        return -1;
    }

    int ret = eXosip_listen_addr(g_excontext_ptr, IPPROTO_UDP,local_ip, local_port, AF_INET, 0);
    if (ret != 0) 
    {
        eXosip_quit(g_excontext_ptr);
        printf( "eXosip_listen_addr failed, ret: %u\n" , ret);
        return -1;
    }

    // / sip 
    std::string user_agent  = "chen/1.0";// Protocol/1.0
    eXosip_set_user_agent(g_excontext_ptr, user_agent.c_str());

	return 0;
}