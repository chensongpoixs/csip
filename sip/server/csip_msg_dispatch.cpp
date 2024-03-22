/***********************************************************************************************
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
#include "csip_msg_dispatch.h"

#define CALLBACK_TEMPLATE(sip_msg, callback) // if (!)


namespace chen {
	csip_msg_dispatch g_sip_msg_dispatch;
	csip_msg_dispatch::csip_msg_dispatch()
	{
	}
	csip_msg_dispatch::~csip_msg_dispatch()
	{
	}
	bool csip_msg_dispatch::init()
	{

		//_register_msg_handler(R2A_BeartHeart, "Render2Auth_BeartHeart", &cwan_session::handler_render_heatbeat);
		_register_msg_handler(EXOSIP_REGISTRATION_SUCCESS, "EXOSIP_REGISTRATION_SUCCESS", &csip_msg_dispatch::handler_exosip_registration_success);
		_register_msg_handler(EXOSIP_REGISTRATION_FAILURE, "EXOSIP_REGISTRATION_FAILURE", &csip_msg_dispatch::handler_exosip_registration_failure);
		_register_msg_handler(EXOSIP_CALL_INVITE, "EXOSIP_CALL_INVITE", &csip_msg_dispatch::handler_exosip_call_invite);
		_register_msg_handler(EXOSIP_CALL_REINVITE, "EXOSIP_CALL_REINVITE", &csip_msg_dispatch::handler_exosip_call_reinvite);
		_register_msg_handler(EXOSIP_CALL_NOANSWER, "EXOSIP_CALL_NOANSWER", &csip_msg_dispatch::handler_exosip_call_noanswer);
		_register_msg_handler(EXOSIP_CALL_PROCEEDING, "EXOSIP_CALL_PROCEEDING", &csip_msg_dispatch::handler_exosip_call_proceeding);
		_register_msg_handler(EXOSIP_CALL_RINGING, "EXOSIP_CALL_RINGING", &csip_msg_dispatch::handler_exosip_call_ringing);
		_register_msg_handler(EXOSIP_CALL_ANSWERED, "EXOSIP_CALL_ANSWERED", &csip_msg_dispatch::handler_exosip_call_answered);
		_register_msg_handler(EXOSIP_CALL_REDIRECTED, "EXOSIP_CALL_REDIRECTED", &csip_msg_dispatch::handler_exosip_call_redirected);
		_register_msg_handler(EXOSIP_CALL_REQUESTFAILURE, "EXOSIP_CALL_REQUESTFAILURE", &csip_msg_dispatch::handler_exosip_call_requestfailure);
		_register_msg_handler(EXOSIP_CALL_SERVERFAILURE, "EXOSIP_CALL_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_call_serverfailure);
		_register_msg_handler(EXOSIP_CALL_GLOBALFAILURE, "EXOSIP_CALL_GLOBALFAILURE", &csip_msg_dispatch::handler_exosip_call_globalfailure);
		_register_msg_handler(EXOSIP_CALL_ACK, "EXOSIP_CALL_ACK", &csip_msg_dispatch::handler_exosip_call_ack);
		_register_msg_handler(EXOSIP_CALL_CANCELLED, "EXOSIP_CALL_CANCELLED", &csip_msg_dispatch::handler_exosip_call_cancelled);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_NEW, "EXOSIP_CALL_MESSAGE_NEW", &csip_msg_dispatch::handler_exosip_call_message_new);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_PROCEEDING, "EXOSIP_CALL_MESSAGE_PROCEEDING", &csip_msg_dispatch::handler_exosip_call_message_proceeding);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_ANSWERED, "EXOSIP_CALL_MESSAGE_ANSWERED", &csip_msg_dispatch::handler_exosip_message_answered);


		_register_msg_handler(EXOSIP_CALL_MESSAGE_REDIRECTED, "EXOSIP_CALL_MESSAGE_REDIRECTED", &csip_msg_dispatch::handler_exosip_call_message_redirected);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_REQUESTFAILURE, "EXOSIP_CALL_MESSAGE_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_call_message_requestfailure);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_SERVERFAILURE, "EXOSIP_CALL_MESSAGE_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_call_message_serverfailure);
		_register_msg_handler(EXOSIP_CALL_MESSAGE_GLOBALFAILURE, "EXOSIP_CALL_MESSAGE_GLOBALFAILURE", &csip_msg_dispatch::handler_exosip_call_message_globalfailure);
		_register_msg_handler(EXOSIP_CALL_CLOSED, "EXOSIP_CALL_CLOSED", &csip_msg_dispatch::handler_exosip_call_closed);
		_register_msg_handler(EXOSIP_CALL_RELEASED, "EXOSIP_CALL_RELEASED", &csip_msg_dispatch::handler_exosip_call_released);
		_register_msg_handler(EXOSIP_MESSAGE_NEW, "EXOSIP_MESSAGE_NEW", &csip_msg_dispatch::handler_exosip_message_new);
		_register_msg_handler(EXOSIP_MESSAGE_PROCEEDING, "EXOSIP_MESSAGE_PROCEEDING", &csip_msg_dispatch::handler_exosip_message_proceeding);
		_register_msg_handler(EXOSIP_MESSAGE_ANSWERED, "EXOSIP_MESSAGE_ANSWERED", &csip_msg_dispatch::handler_exosip_message_answered);
		_register_msg_handler(EXOSIP_MESSAGE_REDIRECTED, "EXOSIP_MESSAGE_REDIRECTED", &csip_msg_dispatch::handler_exosip_message_redirected);
		_register_msg_handler(EXOSIP_MESSAGE_REQUESTFAILURE, "EXOSIP_MESSAGE_REQUESTFAILURE", &csip_msg_dispatch::handler_exosip_message_requestfailure);
		_register_msg_handler(EXOSIP_MESSAGE_SERVERFAILURE, "EXOSIP_MESSAGE_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_message_serverfailure);
		_register_msg_handler(EXOSIP_MESSAGE_GLOBALFAILURE, "EXOSIP_MESSAGE_GLOBALFAILURE", &csip_msg_dispatch::handler_exosip_message_globalfailure);

		_register_msg_handler(EXOSIP_SUBSCRIPTION_NOANSWER, "EXOSIP_SUBSCRIPTION_NOANSWER", &csip_msg_dispatch::handler_exosip_subscription_noanswer);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_PROCEEDING, "EXOSIP_SUBSCRIPTION_PROCEEDING", &csip_msg_dispatch::handler_exosip_subscription_proceeding);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_ANSWERED, "EXOSIP_SUBSCRIPTION_ANSWERED", &csip_msg_dispatch::handler_exosip_subscription_answered);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_REDIRECTED, "EXOSIP_SUBSCRIPTION_REDIRECTED", &csip_msg_dispatch::handler_exosip_subscription_redirected);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_REQUESTFAILURE, "EXOSIP_SUBSCRIPTION_REQUESTFAILURE", &csip_msg_dispatch::handler_exosip_subscription_requestfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_SERVERFAILURE, "EXOSIP_SUBSCRIPTION_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_subscription_serverfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_GLOBALFAILURE, "EXOSIP_SUBSCRIPTION_GLOBALFAILURE", &csip_msg_dispatch::handler_exosip_subscription_globalfailure);
		_register_msg_handler(EXOSIP_SUBSCRIPTION_NOTIFY, "EXOSIP_SUBSCRIPTION_NOTIFY", &csip_msg_dispatch::handler_exosip_subscription_notify);
		_register_msg_handler(EXOSIP_IN_SUBSCRIPTION_NEW, "EXOSIP_IN_SUBSCRIPTION_NEW", &csip_msg_dispatch::handler_exosip_in_subscription_new);
		
		_register_msg_handler(EXOSIP_NOTIFICATION_NOANSWER, "EXOSIP_NOTIFICATION_NOANSWER", &csip_msg_dispatch::handler_exosip_notification_noanswer);
		_register_msg_handler(EXOSIP_NOTIFICATION_PROCEEDING, "EXOSIP_NOTIFICATION_PROCEEDING", &csip_msg_dispatch::handler_exosip_notification_proceeding);
		_register_msg_handler(EXOSIP_NOTIFICATION_ANSWERED, "EXOSIP_NOTIFICATION_ANSWERED", &csip_msg_dispatch::handler_exosip_notification_answered);
		_register_msg_handler(EXOSIP_NOTIFICATION_REDIRECTED, "EXOSIP_NOTIFICATION_REDIRECTED", &csip_msg_dispatch::handler_exosip_notification_redirected);
		_register_msg_handler(EXOSIP_NOTIFICATION_REQUESTFAILURE, "EXOSIP_NOTIFICATION_REQUESTFAILURE", &csip_msg_dispatch::handler_exosip_notification_requestfailure);
		_register_msg_handler(EXOSIP_NOTIFICATION_SERVERFAILURE, "EXOSIP_NOTIFICATION_SERVERFAILURE", &csip_msg_dispatch::handler_exosip_notification_serverfailure);
		_register_msg_handler(EXOSIP_NOTIFICATION_GLOBALFAILURE, "EXOSIP_NOTIFICATION_GLOBALFAILURE", &csip_msg_dispatch::handler_exosip_notification_globalfailure);
		




		return true;
	}
	void csip_msg_dispatch::update(uint32_t DateTime)
	{
	}
	void csip_msg_dispatch::destroy()
	{
	}
	csip_msg_handler* csip_msg_dispatch::get_msg_handler(uint32_t sip_msg_id)
	{
		if (static_cast<int> (sip_msg_id) > EXOSIP_EVENT_COUNT)
		{
			return NULL;
		}

		return &(m_msg_handler[sip_msg_id]);
	}
	bool csip_msg_dispatch::_register_msg_handler(uint32_t sip_event_id, const std::string& msg_name, handler_sip_type handler)
	{
		if ( m_msg_handler[sip_event_id].handler && sip_event_id >  EXOSIP_EVENT_COUNT)
		{
			printf("[%s] register msg error, msg_id = %u, msg_name = %s", __FUNCTION__, sip_event_id, msg_name.c_str());
			return false;
		}
		m_msg_handler[sip_event_id].msg_name = msg_name;//   数据统计
		m_msg_handler[sip_event_id].handle_count = 0;
		m_msg_handler[sip_event_id].handler = handler;
		return true;
	}
}