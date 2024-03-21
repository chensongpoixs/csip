


#ifndef _C_SIP_SERVER_H_
#define _C_SIP_SERVER_H_
#include <mutex>

#include <queue>

#include "eXosip2/eXosip.h"
#include "csafe_queue.h"
#include "csip_event.h"


namespace chen {

	class csip_server
	{
	public:
		explicit csip_server()
			: m_sip_port (0) // 5060
			, m_event_id(0) // 1000000000
			, m_sip_id("")
			, m_sip_host("")
			, m_sip_context_ptr(NULL)
			, m_event_queue(){}
		virtual ~csip_server(){}

	public:
		bool init();

		bool startup(const std::string & user_agent);


		void update(uint32_t DateTime);
		void destroy();


	private:


		csip_event_t* _new_event(eXosip_event_t* exosip_event);
	private:

		int32_t _sip_recv_events();
		int32_t _sip_process_events();
	private:
		uint32_t				m_sip_port;
		uint64_t				m_event_id;
		std::string				m_sip_id;
		std::string				m_sip_host;
		eXosip_t*				m_sip_context_ptr;
		csafe_queue<csip_event_t*>  m_event_queue;
		


		std::thread				m_recv_thread;
		std::thread				m_proc_thread;

	};

	 
}

#endif // _C_SAFE_QUEUE_H_