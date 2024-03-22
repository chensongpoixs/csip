﻿/***********************************************************************************************
			created: 		2019-05-13
			author:			chensong
			purpose:		msg_dipatch
			输赢不重要，答案对你们有什么意义才重要。

			光阴者，百代之过客也，唯有奋力奔跑，方能生风起时，是时代造英雄，英雄存在于时代。或许世人道你轻狂，可你本就年少啊。 看护好，自己的理想和激情。


			我可能会遇到很多的人，听他们讲好2多的故事，我来写成故事或编成歌，用我学来的各种乐器演奏它。
			然后还可能在一个国家遇到一个心仪我的姑娘，她可能会被我帅气的外表捕获，又会被我深邃的内涵吸引，在某个下雨的夜晚，她会全身淋透然后要在我狭小的住处换身上的湿衣服。
			3小时候后她告诉我她其实是这个国家的公主，她愿意向父皇求婚。我不得已告诉她我是穿越而来的男主角，我始终要回到自己的世界。
			然后我的身影慢慢消失，我看到她眼里的泪水，心里却没有任何痛苦，我才知道，原来我的心被丢掉了，我游历全世界的原因，就是要找回自己的本心。
			于是我开始有意寻找各种各样失去心的人，我变成一块砖头，一颗树，一滴水，一朵白云，去听大家为什么会失去自己的本心。
			我发现，刚出生的宝宝，本心还在，慢慢的，他们的本心就会消失，收到了各种黑暗之光的侵蚀。
			从一次争论，到嫉妒和悲愤，还有委屈和痛苦，我看到一只只无形的手，把他们的本心扯碎，蒙蔽，偷走，再也回不到主人都身边。
			我叫他本心猎手。他可能是和宇宙同在的级别 但是我并不害怕，我仔细回忆自己平淡的一生 寻找本心猎手的痕迹。
			沿着自己的回忆，一个个的场景忽闪而过，最后发现，我的本心，在我写代码的时候，会回来。
			安静，淡然，代码就是我的一切，写代码就是我本心回归的最好方式，我还没找到本心猎手，但我相信，顺着这个线索，我一定能顺藤摸瓜，把他揪出来。
************************************************************************************************/

#ifndef _C_SIP_MSG_DISPATCH_H_
#define _C_SIP_MSG_DISPATCH_H_
#include <mutex>

#include <queue>
#include <functional>
#include <memory>
#include "eXosip2/eXosip.h"
#include <unordered_map>
#include "csip_server.h"

#include <map>
#include <atomic>
namespace chen {
	struct csip_event_t;
	class csip_msg_dispatch;

	
	typedef int32_t (csip_msg_dispatch::*handler_sip_type)(const csip_event_t*&   event);
#pragma pack(push, 4)
	struct csip_msg_handler
	{
		std::string							msg_name;
		std::atomic<long>					handle_count;
		handler_sip_type					handler;

		csip_msg_handler() 
			: msg_name("")
			, handle_count(0)
			, handler(NULL) {}
	};
	class csip_msg_dispatch
	{
	private:
		
	public:
		explicit csip_msg_dispatch();

		virtual ~csip_msg_dispatch();
	public:
		bool init();

		void update(uint32_t DateTime);
		void destroy();

	public:

		csip_msg_handler* get_msg_handler(uint32_t sip_msg_id);


	public:
		int32_t handler_exosip_registration_success(const csip_event_t*& event);
		int32_t handler_exosip_registration_failure(const csip_event_t*& event);
		int32_t handler_exosip_call_invite(const csip_event_t*& event);
		int32_t handler_exosip_call_reinvite(const csip_event_t*& event);
		int32_t handler_exosip_call_noanswer(const csip_event_t*& event);
		int32_t handler_exosip_call_proceeding(const csip_event_t*& event);
		int32_t handler_exosip_call_ringing(const csip_event_t*& event);
		int32_t handler_exosip_call_answered(const csip_event_t*& event);
		int32_t handler_exosip_call_redirected(const csip_event_t*& event);
		int32_t handler_exosip_call_requestfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_serverfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_globalfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_ack(const csip_event_t*& event);
		int32_t handler_exosip_call_cancelled(const csip_event_t*& event);
		int32_t handler_exosip_call_message_new(const csip_event_t*& event);
		int32_t handler_exosip_call_message_proceeding(const csip_event_t*& event);
		int32_t handler_exosip_call_message_redirected(const csip_event_t*& event);
		int32_t handler_exosip_call_message_requestfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_message_serverfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_message_globalfailure(const csip_event_t*& event);
		int32_t handler_exosip_call_closed(const csip_event_t*& event);
		int32_t handler_exosip_call_released(const csip_event_t*& event);
		int32_t handler_exosip_message_new(const csip_event_t*& event);
		int32_t handler_exosip_message_proceeding(const csip_event_t*& event);
		int32_t handler_exosip_message_answered(const csip_event_t*& event);
		int32_t handler_exosip_message_redirected(const csip_event_t*& event);
		int32_t handler_exosip_message_requestfailure(const csip_event_t*& event);
		int32_t handler_exosip_message_serverfailure(const csip_event_t*& event);
		int32_t handler_exosip_message_globalfailure(const csip_event_t*& event);
		int32_t handler_exosip_subscription_noanswer(const csip_event_t*& event);
		int32_t handler_exosip_subscription_proceeding(const csip_event_t*& event);
		int32_t handler_exosip_subscription_answered(const csip_event_t*& event);
		int32_t handler_exosip_subscription_redirected(const csip_event_t*& event);
		int32_t handler_exosip_subscription_requestfailure(const csip_event_t*& event);
		int32_t handler_exosip_subscription_serverfailure(const csip_event_t*& event);
		int32_t handler_exosip_subscription_globalfailure(const csip_event_t*& event);
		int32_t handler_exosip_subscription_notify(const csip_event_t*& event);
		int32_t handler_exosip_in_subscription_new(const csip_event_t*& event);
		int32_t handler_exosip_notification_noanswer(const csip_event_t*& event);
		int32_t handler_exosip_notification_proceeding(const csip_event_t*& event);
		int32_t handler_exosip_notification_answered(const csip_event_t*& event);
		int32_t handler_exosip_notification_redirected(const csip_event_t*& event);
		int32_t handler_exosip_notification_requestfailure(const csip_event_t*& event);
		int32_t handler_exosip_notification_serverfailure(const csip_event_t*& event);
		int32_t handler_exosip_notification_globalfailure(const csip_event_t*& event); 
	private:
		bool _register_msg_handler(uint32_t sip_event_id, const std::string & name, handler_sip_type handler);

	private:
		csip_msg_handler		m_msg_handler[EXOSIP_EVENT_COUNT+1];
		//std::map<uint32_t, handler_type>  m_event_map;
	};

#pragma pack(pop)
	extern csip_msg_dispatch g_sip_msg_dispatch;
}


#endif // _C_SIP_MSG_DISPATCH_H_