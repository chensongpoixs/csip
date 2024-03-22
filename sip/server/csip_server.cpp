
 
#include <mutex>

#include <queue>
#include "csip_server.h"
#ifdef _WIN32
/* #include <syslog.h> */
#include <winsock2.h>
#endif
#include <thread>
#include "csip_msg_dispatch.h"

namespace chen {



 
	bool csip_server::init()
	{

		m_sip_context_ptr = eXosip_malloc();
		int32_t ret = ::eXosip_init(m_sip_context_ptr);
		if (ret != 0)
		{
			printf("eXosip init (ret = %u)failed !!!\n", ret);
			return false;
		}

		printf("eXosip_init successfully !!!\n");






		return true;
	}
	bool csip_server::startup(const std::string& user_agent)
	{
		m_sip_host = "192.168.0.191";
		m_sip_port = 8188;
		int32_t ret = ::eXosip_listen_addr(m_sip_context_ptr, IPPROTO_UDP, m_sip_host.c_str(), m_sip_port, AF_INET, 0);
		if (ret != 0)
		{
			printf("eXosip listen addr failed !!!\n");
			return false;
		}

		if (user_agent.length() > 0)
		{
			::eXosip_set_user_agent(m_sip_context_ptr, user_agent.c_str());
		}

		m_recv_thread = std::thread(&csip_server::_sip_recv_events, this);

		m_proc_thread = std::thread(&csip_server::_sip_process_events, this);

		return false;
	}
	void csip_server::update(uint32_t DateTime)
	{
	}
	void csip_server::destroy()
	{
		if (m_recv_thread.joinable())
		{
			m_recv_thread.join();
		}
		if (m_proc_thread.joinable())
		{
			m_proc_thread.join();
		}
		if (m_sip_context_ptr)
		{
			eXosip_quit(m_sip_context_ptr);
			m_sip_context_ptr = NULL;
		}
	}
	  csip_event_t* csip_server::_new_event(eXosip_event_t* exosip_event)
	{

		if (exosip_event == nullptr)
		{
			printf("[%s][%d]return nullptr; exosip event ==null !!!\n", __FUNCTION__, __LINE__);
		}
		if (exosip_event->type < EXOSIP_REGISTRATION_SUCCESS || exosip_event->type > EXOSIP_NOTIFICATION_GLOBALFAILURE)
		{
			return nullptr;
		}

		struct csip_event_t* sip_event_ptr = new struct csip_event_t();
		//sip_event_sptr event(new sip_event_t);// = std::make_shared(SipEvent)();
	//	CEventHandlerManager::EventNameProcPair pair = m_eventHandle.GetEventProc(exosip_event->type);
		/*if (pair.name == nullptr)
			return nullptr;*/

		//csip_msg_handler * handler_ptr = g_sip_msg_dispatch.get_msg_handler(exosip_event->type);

		//if (!handler_ptr)
		//{
		//	printf("[%s][%d] not find sip msg id = %u\n", __FUNCTION__, __LINE__, exosip_event->type );
		//	return NULL;
		//}
		//sip_event_ptr->name = handler_ptr->msg_name.c_str();
		//sip_event_ptr->proc = handler_ptr->handler;
		sip_event_ptr->excontext = m_sip_context_ptr;
		sip_event_ptr->sip_msg_id = exosip_event->type;
		sip_event_ptr->id = m_event_id++;




		return sip_event_ptr;
	}
	int32_t csip_server::_sip_recv_events()
	{
		eXosip_event_t* exosip_event;

		exosip_event = eXosip_event_wait(m_sip_context_ptr, 0, 1);

		//    eXosip_lock(m_excontext);
		//    eXosip_automatic_action(m_excontext);
		//    eXosip_unlock(m_excontext);

		if (exosip_event == nullptr)
			return 0;

		csip_event_t *sipg_event = _new_event(  exosip_event);
		if (!sipg_event)
		{
			printf("[warr ] new event failed !!!!\n");
			return 0;
		 }

		m_event_queue.push(sipg_event);
		printf("Push event:%u,     id= %u  to queue successfully\n",  sipg_event->sip_msg_id, sipg_event->id);
		return 0;
	}
	int32_t csip_server::_sip_process_events()
	{
		const char* event_name;
		uint64_t event_id;

		csip_event_t* sipg_event = NULL;
		if (!m_event_queue.pop(sipg_event))
		{
			return 0;
		}

		//event_name = sipg_event->name;
		//event_id = sipg_event->id;
		//g_sip_msg_dispatch.*(sipg_event->proc)( sipg_event);
		//(m_session_ptr[index].*(handler_ptr->handler)) (p, size);
		//sipg_event->proc((const csip_event_t*)sipg_event);

		csip_msg_handler * handler_ptr = g_sip_msg_dispatch.get_msg_handler(sipg_event->sip_msg_id);

		if (!handler_ptr)
		{
			printf("[%s][%d] not find sip msg id = %u\n", __FUNCTION__, __LINE__, sipg_event->sip_msg_id);
			return NULL;
		}

		if (!handler_ptr->handler)
		{
			printf("[warr] [msg_id = %u][not register]",   sipg_event->sip_msg_id);
			eXosip_event_free(sipg_event->exevent);
			delete sipg_event;
			sipg_event = NULL;
			return 0;
		}
		++handler_ptr->handle_count;
		(g_sip_msg_dispatch.*(handler_ptr->handler)) ((const csip_event_t*&)sipg_event);
		eXosip_event_free(sipg_event->exevent);
		delete sipg_event;
		sipg_event = NULL;
		return 0;
	}
} 