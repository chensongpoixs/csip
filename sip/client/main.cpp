#ifdef _WIN32_WCE
/* #include <syslog.h> */
#include <winsock2.h>
#endif

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
#include <cstdint>
void dt_eXosip_paraseInfoBody(eXosip_t* context, eXosip_event_t* p_event);
typedef struct _device_info/*设备信息结构体*/
{
	char* server_id;/*SIP服务器ID*//*默认值：34020000002000000001*/
	char* server_ip;/*SIP服务器IP地址*//*默认值：192.168.1.178*/
	char* server_port;/*SIP服务器IP端口*//*默认值：5060*/

	char* ipc_id;/*媒体流发送者ID*//*默认值：34020000001180000002*/
	char* ipc_pwd;/*媒体流发送者密码*//*默认值：12345678*/
	char* ipc_ip;/*媒体流发送者IP地址*//*默认值：192.168.1.144*/
	char* ipc_media_port;/*媒体流发送者IP端口*//*默认值：20000*/
	char* ipc_sess_port; /*会话端口，即SIP端口*/

	char* alarm_id; /*报警器ID*/

	char* media_ip;
	char* media_port;

	char* device_name;/*设备/区域/系统名称*//*默认值：IPC*/
	char* device_manufacturer;/*设备厂商*//*默认值：CSENN*/
	char* device_model;/*设备型号*//*默认值：GB28181*/
	char* device_firmware;/*设备固件版本*//*默认值：V1.0*/
	char* device_encode;/*是否编码*//*取值范围：ON/OFF*//*默认值：ON*/
	char* device_record;/*是否录像*//*取值范围：ON/OFF*//*默认值：OFF*/
}DEVICE_INFO;
typedef struct _device_status/*设备状态结构体*/
{
	char* status_on;/*设备打开状态*//*取值范围：ON/OFF*//*默认值：ON*/
	char* status_ok;/*是否正常工作*//*取值范围：OK/ERROR*//*默认值：OFF*/
	char* status_online;/*是否在线*//*取值范围：ONLINE/OFFLINE*//*默认值：ONLINE*/
	char* status_guard;/*布防状态*//*取值范围：ONDUTY/OFFDUTY/ALARM*//*默认值：OFFDUTY*/
	char* status_time;/*设备日期和时间*//*格式：xxxx-xx-xxTxx:xx:xx*//*默认值：2012-12-20T12:12:20*/
}DEVICE_STATUS;

int InitiateWinsock()
{
	int Error;
	WORD Version = MAKEWORD(2, 2);
	WSADATA WsaData;
	Error = WSAStartup(Version, &WsaData);	      //Start up WSA	  
	if (Error != 0)
		return 0;
	else
	{
		if (LOBYTE(WsaData.wVersion) != 2 || HIBYTE(WsaData.wHighVersion) != 2)
		{
			WSACleanup();
			return 0;
		}

	}
	return 1;
}

static eXosip_t* g_context_sip_ptr;

static DEVICE_INFO device_info;
DEVICE_STATUS device_status;


BOOL SenAliveFlag = FALSE;
int g_register_id = 0;/*注册ID/用来更新注册或取消注册*/
int g_call_id = 0;/*INVITE连接ID/用来分辨不同的INVITE连接，每个时刻只允许有一个INVITE连接*/
int g_did_realPlay = 0;/*会话ID/用来分辨不同的会话：实时视音频点播*/
int g_did_backPlay = 0;/*会话ID/用来分辨不同的会话：历史视音频回放*/
int g_did_fileDown = 0;/*会话ID/用来分辨不同的会话：视音频文件下载*/

bool eXosip_initial(void)
{
	InitiateWinsock();
	g_context_sip_ptr = eXosip_malloc();
	if (eXosip_init(g_context_sip_ptr))
	{
		printf("[exosip init failed !!!][]\n");
		return false;
		// syslog_wrapper(LOG_ERR, "eXosip_init failed");
		 //exit(1);
	}
	printf("[exosip init ok !!!]\n");
	return true;
	//eXosip_init(); // 初始化eXosip库
	// ...
	return 0;
}

//注册一个SIP用户：
int eXosip_register(const char* username, const char* realm, const char* contact, const char* proxy)
{
	int id = 45;
	osip_message_t* reg = NULL;
	eXosip_call_get_reference(g_context_sip_ptr, id); // 获取注册请求的eXosip引用
	osip_message_set_header(reg, "Via", "SIP/2.0/TCP %s"/*, host*/); // 设置注册请求的Via头
	osip_message_set_header(reg, "From", "<sip:%s>%s;tag=%s"/*, username, realm, tag*/); // 设置From头
	osip_message_set_header(reg, "To", "<sip:%s>%s"/*, username, realm*/); // 设置To头
	osip_message_set_header(reg, "Contact", "<sip:%s>%s"/*, contact, realm*/); // 设置Contact头
	eXosip_call_send_request(g_context_sip_ptr, id, reg); // 发送注册请求
	// ...
	return 0;
}


//EXOSIP_REGISTRATION_FAILURE-->EXOSIP_REGISTRATION_SUCCESS
int dt_eXosip_register(eXosip_t* context, int expires)  /*expires/注册消息过期时间，单位为秒*/
{
	int ret = 0;
	eXosip_event_t* je = NULL;
	osip_message_t* reg = NULL;
	char from[1024] = { 0 };/*sip:主叫用户名@被叫IP地址*/
	char proxy[1024] = { 0 };/*sip:被叫IP地址:被叫IP端口*/


	sprintf_s(from, sizeof(from), "sip:%s@%s:%s", device_info.ipc_id, device_info.ipc_ip, device_info.ipc_sess_port);
	printf("%s\n", from);
	sprintf_s(proxy, sizeof(proxy), "sip:%s@%s:%s", device_info.server_id, device_info.server_ip, device_info.server_port);
	printf("proxy:%s\n", proxy);

	/*发送不带认证信息的注册请求*/
retry:
	eXosip_masquerade_contact(context, device_info.ipc_ip, atoi(device_info.ipc_sess_port));
	eXosip_set_user_agent(context, "SipReg v1.0");
	// 	if (eXosip_add_authentication_info (context, device_info.ipc_id, device_info.ipc_id, device_info.ipc_pwd, "MD5", NULL)) {
	// 		dbg("eXosip_add_authentication_info failed");
	// 		exit (1);
	// 	}
	eXosip_lock(context);
	g_register_id = eXosip_register_build_initial_register(context, from, proxy, NULL, expires, &reg);
	//osip_message_set_authorization(reg, "Capability algorithm=\"H:MD5\"");  
	if (0 > g_register_id)
	{
		eXosip_unlock(context);
		printf("eXosip_register_build_initial_register error!\r\n");
		system("pause"); return -1;
	}
	printf("eXosip_register_build_initial_register success!\r\n");

	ret = eXosip_register_send_register(context, g_register_id, reg);
	eXosip_unlock(context);
	if (0 != ret)
	{
		printf("eXosip_register_send_register no authorization error!\r\n");
		system("pause"); return -1;
	}
	printf("eXosip_register_send_register no authorization success!\r\n");

	printf("g_register_id=%d\r\n", g_register_id);

	for (;;)
	{
		je = eXosip_event_wait(context, 0, 50);/*侦听消息的到来*/
		if (NULL == je)/*没有接收到消息*/
		{
			eXosip_execute(context);
			eXosip_automatic_action(context);
			osip_usleep(1000);
			continue;
		}
		if (EXOSIP_REGISTRATION_FAILURE == je->type)/*注册失败*/
		{
			printf("<EXOSIP_REGISTRATION_FAILURE>\r\n");
			printf("je->rid=%d\r\n", je->rid);
			/*收到服务器返回的注册失败/401未认证状态*/
			if ((NULL != je->response) && (401 == je->response->status_code))
			{
				reg = NULL;
				/*发送携带认证信息的注册请求*/
				eXosip_lock(context);
				eXosip_clear_authentication_info(context);/*清除认证信息*/
				eXosip_add_authentication_info(context, device_info.ipc_id, device_info.ipc_id, device_info.ipc_pwd, "MD5", NULL);/*添加主叫用户的认证信息*/
				//eXosip_add_authentication_info(context, device_info.ipc_id, device_info.ipc_id, device_info.ipc_pwd, NULL, NULL);
				eXosip_register_build_register(context, je->rid, expires, &reg);
				ret = eXosip_register_send_register(context, je->rid, reg);
				eXosip_unlock(context);
				if (0 != ret)
				{
					printf("eXosip_register_send_register authorization error!\r\n");
					system("pause"); return -1;
				}
				printf("eXosip_register_send_register authorization success!\r\n");
			}
			else/*真正的注册失败*/
			{
				printf("EXOSIP_REGISTRATION_FAILURE error!\r\n");
				Sleep(3000);
				goto retry;/*重新注册*/
			}
		}
		else if (EXOSIP_REGISTRATION_SUCCESS == je->type)
		{
			/*收到服务器返回的注册成功*/
			eXosip_execute(context);
			eXosip_automatic_action(context);
			printf("<EXOSIP_REGISTRATION_SUCCESS>\r\n");
			g_register_id = je->rid;/*保存注册成功的注册ID*/
			printf("g_register_id=%d\r\n", g_register_id);
			break;
		}
	}
	eXosip_event_free(je);
	return 0;
}




void dt_eXosip_keepAlive(eXosip_t* context)
{

	osip_message_t* rqt_msg = NULL;
	char to[100];/*sip:主叫用户名@被叫IP地址*/
	char from[100];/*sip:被叫IP地址:被叫IP端口*/
	char xml_body[4096];

	memset(to, 0, 100);
	memset(from, 0, 100);
	memset(xml_body, 0, 4096);
	sprintf_s(to, sizeof(to), "sip:%s@%s:%s", device_info.server_id, device_info.server_ip, device_info.server_port);
	sprintf_s(from, sizeof(from), "sip:%s@%s:%s", device_info.ipc_id, device_info.ipc_ip, device_info.ipc_sess_port);
	eXosip_message_build_request(context, &rqt_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
	_snprintf_s(xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
		"<Notify>\r\n"
		"<CmdType>Keepalive</CmdType>\r\n"/*命令类型*/
		"<SN>1</SN>\r\n"/*命令序列号*/
		"<DeviceID>%s</DeviceID>\r\n"/*设备编码*/
		"<Status>OK</Status>\r\n"/*是否正常工作*/
		"</Notify>\r\n",
		device_info.ipc_id);
	osip_message_set_body(rqt_msg, xml_body, strlen(xml_body));
	osip_message_set_content_type(rqt_msg, "Application/MANSCDP+xml");
	eXosip_lock(context);
	eXosip_message_send_request(context, rqt_msg);/*回复"MESSAGE"请求*/
	eXosip_unlock(context);
	printf("dt_eXosip_sendKeepAlive success!\r\n");
}

/*检测并打印事件*/
void dt_eXosip_printEvent(eXosip_t* context, eXosip_event_t* p_event)
{
	osip_message_t* clone_event = NULL;
	size_t length = 0;
	char* message = NULL;

	printf("\r\n##############################################################\r\n");
	switch (p_event->type)
	{
		// 	case EXOSIP_REGISTRATION_NEW:
		// 		dbg("EXOSIP_REGISTRATION_NEW\r\n");
		// 		break;
	case EXOSIP_REGISTRATION_SUCCESS:
		printf("EXOSIP_REGISTRATION_SUCCESS\r\n");
		break;
	case EXOSIP_REGISTRATION_FAILURE:
		printf("EXOSIP_REGISTRATION_FAILURE\r\n");
		break;
		// 	case EXOSIP_REGISTRATION_REFRESHED:
		// 		dbg("EXOSIP_REGISTRATION_REFRESHED\r\n");
		// 		break;
		// 	case EXOSIP_REGISTRATION_TERMINATED:
		// 		dbg("EXOSIP_REGISTRATION_TERMINATED\r\n");
		// 		break;
	case EXOSIP_CALL_INVITE:
		printf("EXOSIP_CALL_INVITE\r\n");
		break;
	case EXOSIP_CALL_REINVITE:
		printf("EXOSIP_CALL_REINVITE\r\n");
		break;
	case EXOSIP_CALL_NOANSWER:
		printf("EXOSIP_CALL_NOANSWER\r\n");
		break;
	case EXOSIP_CALL_PROCEEDING:
		printf("EXOSIP_CALL_PROCEEDING\r\n");
		break;
	case EXOSIP_CALL_RINGING:
		printf("EXOSIP_CALL_RINGING\r\n");
		break;
	case EXOSIP_CALL_ANSWERED:
		printf("EXOSIP_CALL_ANSWERED\r\n");
		break;
	case EXOSIP_CALL_REDIRECTED:
		printf("EXOSIP_CALL_REDIRECTED\r\n");
		break;
	case EXOSIP_CALL_REQUESTFAILURE:
		printf("EXOSIP_CALL_REQUESTFAILURE\r\n");
		break;
	case EXOSIP_CALL_SERVERFAILURE:
		printf("EXOSIP_CALL_SERVERFAILURE\r\n");
		break;
	case EXOSIP_CALL_GLOBALFAILURE:
		printf("EXOSIP_CALL_GLOBALFAILURE\r\n");
		break;
	case EXOSIP_CALL_ACK:
		printf("EXOSIP_CALL_ACK\r\n");
		break;
	case EXOSIP_CALL_CANCELLED:
		printf("EXOSIP_CALL_CANCELLED\r\n");
		break;
		// 	case EXOSIP_CALL_TIMEOUT:
		// 		dbg("EXOSIP_CALL_TIMEOUT\r\n");
		// 		break;
	case EXOSIP_CALL_MESSAGE_NEW:
		printf("EXOSIP_CALL_MESSAGE_NEW\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_PROCEEDING:
		printf("EXOSIP_CALL_MESSAGE_PROCEEDING\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_ANSWERED:
		printf("EXOSIP_CALL_MESSAGE_ANSWERED\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_REDIRECTED:
		printf("EXOSIP_CALL_MESSAGE_REDIRECTED\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
		printf("EXOSIP_CALL_MESSAGE_REQUESTFAILURE\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_SERVERFAILURE:
		printf("EXOSIP_CALL_MESSAGE_SERVERFAILURE\r\n");
		break;
	case EXOSIP_CALL_MESSAGE_GLOBALFAILURE:
		printf("EXOSIP_CALL_MESSAGE_GLOBALFAILURE\r\n");
		break;
	case EXOSIP_CALL_CLOSED:
		printf("EXOSIP_CALL_CLOSED\r\n");
		break;
	case EXOSIP_CALL_RELEASED:
		printf("EXOSIP_CALL_RELEASED\r\n");
		break;
	case EXOSIP_MESSAGE_NEW:
		printf("EXOSIP_MESSAGE_NEW\r\n");
		break;
	case EXOSIP_MESSAGE_PROCEEDING:
		printf("EXOSIP_MESSAGE_PROCEEDING\r\n");
		break;
	case EXOSIP_MESSAGE_ANSWERED:
		printf("EXOSIP_MESSAGE_ANSWERED\r\n");
		break;
	case EXOSIP_MESSAGE_REDIRECTED:
		printf("EXOSIP_MESSAGE_REDIRECTED\r\n");
		break;
	case EXOSIP_MESSAGE_REQUESTFAILURE:
		printf("EXOSIP_MESSAGE_REQUESTFAILURE\r\n");
		break;
	case EXOSIP_MESSAGE_SERVERFAILURE:
		printf("EXOSIP_MESSAGE_SERVERFAILURE\r\n");
		break;
	case EXOSIP_MESSAGE_GLOBALFAILURE:
		printf("EXOSIP_MESSAGE_GLOBALFAILURE\r\n");
		break;
		// 	case EXOSIP_SUBSCRIPTION_UPDATE:
		// 		dbg("EXOSIP_SUBSCRIPTION_UPDATE\r\n");
		// 		break;
		// 	case EXOSIP_SUBSCRIPTION_CLOSED:
		// 		dbg("EXOSIP_SUBSCRIPTION_CLOSED\r\n");
		// 		break;
	case EXOSIP_SUBSCRIPTION_NOANSWER:
		printf("EXOSIP_SUBSCRIPTION_NOANSWER\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_PROCEEDING:
		printf("EXOSIP_SUBSCRIPTION_PROCEEDING\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_ANSWERED:
		printf("EXOSIP_SUBSCRIPTION_ANSWERED\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_REDIRECTED:
		printf("EXOSIP_SUBSCRIPTION_REDIRECTED\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_REQUESTFAILURE:
		printf("EXOSIP_SUBSCRIPTION_REQUESTFAILURE\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_SERVERFAILURE:
		printf("EXOSIP_SUBSCRIPTION_SERVERFAILURE\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_GLOBALFAILURE:
		printf("EXOSIP_SUBSCRIPTION_GLOBALFAILURE\r\n");
		break;
	case EXOSIP_SUBSCRIPTION_NOTIFY:
		printf("EXOSIP_SUBSCRIPTION_NOTIFY\r\n");
		break;
		// 	case EXOSIP_SUBSCRIPTION_RELEASED:
		// 		dbg("EXOSIP_SUBSCRIPTION_RELEASED\r\n");
		// 		break;
	case EXOSIP_IN_SUBSCRIPTION_NEW:
		printf("EXOSIP_IN_SUBSCRIPTION_NEW\r\n");
		break;
		// 	case EXOSIP_IN_SUBSCRIPTION_RELEASED:
		// 		dbg("EXOSIP_IN_SUBSCRIPTION_RELEASED\r\n");
		// 		break;
	case EXOSIP_NOTIFICATION_NOANSWER:
		printf("EXOSIP_NOTIFICATION_NOANSWER\r\n");
		break;
	case EXOSIP_NOTIFICATION_PROCEEDING:
		printf("EXOSIP_NOTIFICATION_PROCEEDING\r\n");
		break;
	case EXOSIP_NOTIFICATION_ANSWERED:
		printf("EXOSIP_NOTIFICATION_ANSWERED\r\n");
		break;
	case EXOSIP_NOTIFICATION_REDIRECTED:
		printf("EXOSIP_NOTIFICATION_REDIRECTED\r\n");
		break;
	case EXOSIP_NOTIFICATION_REQUESTFAILURE:
		printf("EXOSIP_NOTIFICATION_REQUESTFAILURE\r\n");
		break;
	case EXOSIP_NOTIFICATION_SERVERFAILURE:
		printf("EXOSIP_NOTIFICATION_SERVERFAILURE\r\n");
		break;
	case EXOSIP_NOTIFICATION_GLOBALFAILURE:
		printf("EXOSIP_NOTIFICATION_GLOBALFAILURE\r\n");
		break;
	case EXOSIP_EVENT_COUNT:
		printf("EXOSIP_EVENT_COUNT\r\n");
		break;
	default:
		printf("..................\r\n");
		break;
	}
	osip_message_clone(p_event->request, &clone_event);
	osip_message_to_str(clone_event, &message, &length);
	printf("%s\r\n", message);
	printf("##############################################################\r\n\r\n");
}
/*解析INVITE的SDP消息体，同时保存全局INVITE连接ID和全局会话ID*/
void dt_eXosip_paraseInviteBody(eXosip_t* context, eXosip_event_t* p_event)
{
	sdp_message_t* sdp_msg = NULL;
	char* media_sever_ip = NULL;
	char* media_sever_port = NULL;

	sdp_msg = eXosip_get_remote_sdp(context, p_event->did);
	if (sdp_msg == NULL)
	{
		printf("eXosip_get_remote_sdp NULL!\r\n");
		return;
	}
	printf("eXosip_get_remote_sdp success!\r\n");

	g_call_id = p_event->cid;/*保存全局INVITE连接ID*/
	/*实时点播*/
	if (0 == strcmp(sdp_msg->s_name, "Play"))
	{
		g_did_realPlay = p_event->did;/*保存全局会话ID：实时视音频点播*/
	}
	/*回放*/
	else if (0 == strcmp(sdp_msg->s_name, "Playback"))
	{
		g_did_backPlay = p_event->did;/*保存全局会话ID：历史视音频回放*/
	}
	/*下载*/
	else if (0 == strcmp(sdp_msg->s_name, "Download"))
	{
		g_did_fileDown = p_event->did;/*保存全局会话ID：视音频文件下载*/
	}
	/*从SIP服务器发过来的INVITE请求的o字段或c字段中获取媒体服务器的IP地址与端口*/
	media_sever_ip = sdp_message_o_addr_get(sdp_msg);/*媒体服务器IP地址*/
	media_sever_port = sdp_message_m_port_get(sdp_msg, 0);/*媒体服务器IP端口*/
	printf("%s->%s:%s\r\n", sdp_msg->s_name, media_sever_ip, media_sever_port);
	//dt_eXosip_callback.dt_eXosip_mediaControl(sdp_msg->s_name, media_sever_ip, media_sever_port);
}


/*解析MESSAGE的XML消息体*/
void dt_eXosip_paraseMsgBody(eXosip_t* context, eXosip_event_t* p_event)
{
	/*与请求相关的变量*/
	osip_body_t* p_rqt_body = NULL;
	char* p_xml_body = NULL;
	char* p_str_begin = NULL;
	char* p_str_end = NULL;
	char xml_cmd_type[20];
	char xml_cmd_sn[10];
	char xml_device_id[30];
	char xml_command[30];
	/*与回复相关的变量*/
	osip_message_t* rsp_msg = NULL;
	char to[100];/*sip:主叫用户名@被叫IP地址*/
	char from[100];/*sip:被叫IP地址:被叫IP端口*/
	char rsp_xml_body[4096];

	memset(xml_cmd_type, 0, 20);
	memset(xml_cmd_sn, 0, 10);
	memset(xml_device_id, 0, 30);
	memset(xml_command, 0, 30);
	memset(to, 0, 100);
	memset(from, 0, 100);
	memset(rsp_xml_body, 0, 4096);

	sprintf_s(to, sizeof(to), "sip:%s@%s:%s", device_info.server_id, device_info.server_ip, device_info.server_port);
	sprintf_s(from, sizeof(from), "sip:%s@%s:%s", device_info.ipc_id, device_info.ipc_ip, device_info.ipc_sess_port);
	eXosip_message_build_request(context, &rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/

	osip_message_get_body(p_event->request, 0, &p_rqt_body);/*获取接收到请求的XML消息体*/
	if (NULL == p_rqt_body)
	{
		printf("osip_message_get_body null!\r\n");
		return;
	}
	p_xml_body = p_rqt_body->body;
	printf("osip_message_get_body success!\r\n");

	printf("**********CMD START**********\r\n");
	p_str_begin = strstr(p_xml_body, "<CmdType>");/*查找字符串"<CmdType>"*/
	p_str_end = strstr(p_xml_body, "</CmdType>");
	memcpy(xml_cmd_type, p_str_begin + 9, p_str_end - p_str_begin - 9);/*保存<CmdType>到xml_cmd_type*/
	printf("<CmdType>:%s\r\n", xml_cmd_type);

	p_str_begin = strstr(p_xml_body, "<SN>");/*查找字符串"<SN>"*/
	p_str_end = strstr(p_xml_body, "</SN>");
	memcpy(xml_cmd_sn, p_str_begin + 4, p_str_end - p_str_begin - 4);/*保存<SN>到xml_cmd_sn*/
	printf("<SN>:%s\r\n", xml_cmd_sn);

	p_str_begin = strstr(p_xml_body, "<DeviceID>");/*查找字符串"<DeviceID>"*/
	p_str_end = strstr(p_xml_body, "</DeviceID>");
	memcpy(xml_device_id, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存<DeviceID>到xml_device_id*/
	printf("<DeviceID>:%s\r\n", xml_device_id);
	printf("***********CMD END***********\r\n");

	if (0 == strcmp(xml_cmd_type, "DeviceControl"))/*设备控制*/
	{
		printf("**********CONTROL START**********\r\n");
		/*向左、向右、向上、向下、放大、缩小、停止遥控*/
		p_str_begin = strstr(p_xml_body, "<PTZCmd>");/*查找字符串"<PTZCmd>"*/
		if (NULL != p_str_begin)
		{
			p_str_end = strstr(p_xml_body, "</PTZCmd>");
			memcpy(xml_command, p_str_begin + 8, p_str_end - p_str_begin - 8);/*保存<PTZCmd>到xml_command*/
			printf("<PTZCmd>:%s\r\n", xml_command);
			goto DeviceControl_Next;
		}
		/*开始手动录像、停止手动录像*/
		p_str_begin = strstr(p_xml_body, "<RecordCmd>");/*查找字符串"<RecordCmd>"*/
		if (NULL != p_str_begin)
		{
			p_str_end = strstr(p_xml_body, "</RecordCmd>");
			memcpy(xml_command, p_str_begin + 11, p_str_end - p_str_begin - 11);/*保存<RecordCmd>到xml_command*/
			printf("<RecordCmd>:%s\r\n", xml_command);
			goto DeviceControl_Next;
		}
		/*布防、撤防*/
		p_str_begin = strstr(p_xml_body, "<GuardCmd>");/*查找字符串"<GuardCmd>"*/
		if (NULL != p_str_begin)
		{
			p_str_end = strstr(p_xml_body, "</GuardCmd>");
			memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存<GuardCmd>到xml_command*/
			printf("<GuardCmd>:%s\r\n", xml_command);
			goto DeviceControl_Next;
		}
		/*报警复位：30秒内不再触发报警*/
		p_str_begin = strstr(p_xml_body, "<AlarmCmd>");/*查找字符串"<AlarmCmd>"*/
		if (NULL != p_str_begin)
		{
			p_str_end = strstr(p_xml_body, "</AlarmCmd>");
			memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存<AlarmCmd>到xml_command*/
			printf("<AlarmCmd>:%s\r\n", xml_command);
			goto DeviceControl_Next;
		}
		/*设备远程启动*/
		p_str_begin = strstr(p_xml_body, "<TeleBoot>");/*查找字符串"<TeleBoot>"*/
		if (NULL != p_str_begin)
		{
			p_str_end = strstr(p_xml_body, "</TeleBoot>");
			memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存<TeleBoot>到xml_command*/
			printf("<TeleBoot>:%s\r\n", xml_command);
			goto DeviceControl_Next;
		}
	DeviceControl_Next:
		printf("***********CONTROL END***********\r\n");
		//if (0 == strcmp(xml_command, "A50F01021F0000D6"))/*向左*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_LEFT);//调用dt_eXosip_deviceControl函数
		//}
		//else if (0 == strcmp(xml_command, "A50F01011F0000D5"))/*向右*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_RIGHT);
		//}
		//else if (0 == strcmp(xml_command, "A50F0108001F00DC"))/*向上*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_UP);
		//}
		//else if (0 == strcmp(xml_command, "A50F0104001F00D8"))/*向下*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
		//}
		//else if (0 == strcmp(xml_command, "A50F0110000010D5"))/*放大*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_LARGE);
		//}
		//else if (0 == strcmp(xml_command, "A50F0120000010E5"))/*缩小*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_SMALL);
		//}
		//else if (0 == strcmp(xml_command, "A50F0100000000B5"))/*停止遥控*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
		//}
		//else if (0 == strcmp(xml_command, "Record"))/*开始手动录像*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_REC_START);
		//}
		//else if (0 == strcmp(xml_command, "StopRecord"))/*停止手动录像*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_REC_STOP);
		//}
		//else if (0 == strcmp(xml_command, "SetGuard"))/*布防*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_GUD_START);
		//	strcpy_s(device_status.status_guard, strlen("ONDUTY"), "ONDUTY");/*设置布防状态为"ONDUTY"*/
		//}
		//else if (0 == strcmp(xml_command, "ResetGuard"))/*撤防*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_GUD_STOP);
		//	strcpy_s(device_status.status_guard, strlen("OFFDUTY"), "OFFDUTY");/*设置布防状态为"OFFDUTY"*/
		//}
		//else if (0 == strcmp(xml_command, "ResetAlarm"))/*报警复位*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_ALM_RESET);
		//}
		//else if (0 == strcmp(xml_command, "Boot"))/*设备远程启动*/
		//{
		//	dt_eXosip_callback.dt_eXosip_deviceControl(EXOSIP_CTRL_TEL_BOOT);
		//}
		//else
		{
			printf("unknown device control command!\r\n");
		}
		_snprintf_s(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
			"<Response>\r\n"
			"<CmdType>DeviceControl</CmdType>\r\n"/*命令类型*/
			"<SN>%s</SN>\r\n"/*命令序列号*/
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统编码*/
			"<Result>OK</Result>\r\n"/*执行结果标志*/
			"</Response>\r\n",
			xml_cmd_sn,
			xml_device_id);
	}
	else if (0 == strcmp(xml_cmd_type, "Alarm"))/*报警事件通知和分发：报警通知响应*/
	{
		printf("**********ALARM START**********\r\n");
		/*报警通知响应*/
		printf("local eventAlarm response success!\n");
		printf("***********ALARM END***********\r\n");
		return;
	}
	else if (0 == strcmp(xml_cmd_type, "Catalog"))/*网络设备信息查询：设备目录查询*/
	{
		printf("**********CATALOG START**********\r\n");
		/*设备目录查询*/
		printf("***********CATALOG END***********\r\n");
		_snprintf_s(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
			"<Response>\r\n"
			"<CmdType>Catalog</CmdType>\r\n"/*命令类型*/
			"<SN>%s</SN>\r\n"/*命令序列号*/
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			"<SumNum>2</SumNum>\r\n"/*查询结果总数*/
			"<DeviceList Num=\"2\">\r\n"/*设备目录项列表*/
			"<Item>\r\n"
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			"<Name>%s</Name>\r\n"/*设备/区域/系统名称*/
			"<Manufacturer>%s</Manufacturer>\r\n"/*设备厂商*/
			"<Model>%s</Model>\r\n"/*设备型号*/
			"<Owner>Owner</Owner>\r\n"/*设备归属*/
			"<CivilCode>CivilCode</CivilCode>\r\n"/*行政区域*/
			"<Address>Address</Address>\r\n"/*安装地址*/
			"<Parental>0</Parental>\r\n"/*是否有子设备*/
			"<SafetyWay>0</SafetyWay>\r\n"/*信令安全模式/0为不采用/2为S/MIME签名方式/3为S/MIME加密签名同时采用方式/4为数字摘要方式*/
			"<RegisterWay>1</RegisterWay>\r\n"/*注册方式/1为符合sip3261标准的认证注册模式/2为基于口令的双向认证注册模式/3为基于数字证书的双向认证注册模式*/
			"<Secrecy>0</Secrecy>\r\n"
			"<Status>ON</Status>\r\n"
			"</Item>\r\n"
			"<Item>\r\n"
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			"<Name></Name>\r\n"/*设备/区域/系统名称*/
			"<Manufacturer>%s</Manufacturer>\r\n"/*设备厂商*/
			"<Model>AlarmIn</Model>\r\n"/*设备型号*/
			"<Owner>Owner</Owner>\r\n"/*设备归属*/
			"<CivilCode>CivilCode</CivilCode>\r\n"/*行政区域*/
			"<Address>Address</Address>\r\n"/*安装地址*/
			"<Parental>0</Parental>\r\n"/*是否有子设备*/
			"<SafetyWay>0</SafetyWay>\r\n"/*信令安全模式/0为不采用/2为S/MIME签名方式/3为S/MIME加密签名同时采用方式/4为数字摘要方式*/
			"<RegisterWay>1</RegisterWay>\r\n"/*注册方式/1为符合sip3261标准的认证注册模式/2为基于口令的双向认证注册模式/3为基于数字证书的双向认证注册模式*/
			"<Secrecy>0</Secrecy>\r\n"
			"<Status>ON</Status>\r\n"
			"</Item>\r\n"
			"</DeviceList>\r\n"
			"</Response>\r\n",
			xml_cmd_sn,
			xml_device_id,
			xml_device_id,
			device_info.device_name,
			device_info.device_manufacturer,
			device_info.device_model,
			device_info.alarm_id,
			device_info.device_manufacturer);
	}
	else if (0 == strcmp(xml_cmd_type, "DeviceInfo"))/*网络设备信息查询：设备信息查询*/
	{
		printf("**********DEVICE INFO START**********\r\n");
		/*设备信息查询*/
		printf("***********DEVICE INFO END***********\r\n");
		_snprintf_s(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
			"<Response>\r\n"
			"<CmdType>DeviceInfo</CmdType>\r\n"/*命令类型*/
			"<SN>%s</SN>\r\n"/*命令序列号*/
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			"<Result>OK</Result>\r\n"/*查询结果*/
			"<DeviceType>IPC</DeviceType>\r\n"
			"<Manufacturer>%s</Manufacturer>\r\n"/*设备生产商*/
			"<Model>%s</Model>\r\n"/*设备型号*/
			"<Firmware>%s</Firmware>\r\n"/*设备固件版本*/
			"<MaxCamera>1</MaxCamera>\r\n"
			"<MaxAlarm>1</MaxAlarm>\r\n"
			"</Response>\r\n",
			xml_cmd_sn,
			xml_device_id,
			device_info.device_manufacturer,
			device_info.device_model,
			device_info.device_firmware);
	}
	else if (0 == strcmp(xml_cmd_type, "DeviceStatus"))/*网络设备信息查询：设备状态查询*/
	{
		printf("**********DEVICE STATUS START**********\r\n");
		/*设备状态查询*/
		printf("***********DEVICE STATUS END***********\r\n");
		char xml_status_guard[10];
		strcpy_s(xml_status_guard, sizeof(xml_status_guard), device_status.status_guard);/*保存当前布防状态*/
		//dt_eXosip_callback.dt_eXosip_getDeviceStatus(&device_status);/*获取设备当前状态*/
		strcpy_s(device_status.status_guard, sizeof(xml_status_guard), xml_status_guard);/*恢复当前布防状态*/
		_snprintf_s(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
			"<Response>\r\n"
			"<CmdType>DeviceStatus</CmdType>\r\n"/*命令类型*/
			"<SN>%s</SN>\r\n"/*命令序列号*/
			"<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			"<Result>OK</Result>\r\n"/*查询结果标志*/
			"<Online>%s</Online>\r\n"/*是否在线/ONLINE/OFFLINE*/
			"<Status>%s</Status>\r\n"/*是否正常工作*/
			"<Encode>%s</Encode>\r\n"/*是否编码*/
			"<Record>%s</Record>\r\n"/*是否录像*/
			"<DeviceTime>%s</DeviceTime>\r\n"/*设备时间和日期*/
			"<Alarmstatus Num=\"1\">\r\n"/*报警设备状态列表*/
			"<Item>\r\n"
			"<DeviceID>%s</DeviceID>\r\n"/*报警设备编码*/
			"<DutyStatus>%s</DutyStatus>\r\n"/*报警设备状态*/
			"</Item>\r\n"
			"</Alarmstatus>\r\n"
			"</Response>\r\n",
			xml_cmd_sn,
			xml_device_id,
			device_status.status_online,
			device_status.status_ok,
			device_info.device_encode,
			device_info.device_record,
			device_status.status_time,
			xml_device_id,
			device_status.status_guard);
	}
	else if (0 == strcmp(xml_cmd_type, "RecordInfo"))/*设备视音频文件检索*/
	{
		/*录像文件检索*/
		char xml_file_path[30];
		char xml_start_time[30];
		char xml_end_time[30];
		char xml_recorder_id[30];
		char item_start_time[30];
		char item_end_time[30];
		char rsp_item_body[4096];
		int  record_list_num = 0;
		int  record_list_ret = 0;

		memset(xml_file_path, 0, 30);
		memset(xml_start_time, 0, 30);
		memset(xml_end_time, 0, 30);
		memset(xml_recorder_id, 0, 30);
		memset(item_start_time, 0, 30);
		memset(item_end_time, 0, 30);
		memset(rsp_item_body, 0, 4096);
		printf("**********RECORD INFO START**********\r\n");
		p_str_begin = strstr(p_xml_body, "<FilePath>");/*查找字符串"<FilePath>"*/
		p_str_end = strstr(p_xml_body, "</FilePath>");
		memcpy(xml_file_path, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存<FilePath>到xml_file_path*/
		printf("<FilePath>:%s\r\n", xml_file_path);

		p_str_begin = strstr(p_xml_body, "<StartTime>");/*查找字符串"<StartTime>"*/
		p_str_end = strstr(p_xml_body, "</StartTime>");
		memcpy(xml_start_time, p_str_begin + 11, p_str_end - p_str_begin - 11);/*保存<StartTime>到xml_start_time*/
		printf("<StartTime>:%s\r\n", xml_start_time);

		p_str_begin = strstr(p_xml_body, "<EndTime>");/*查找字符串"<EndTime>"*/
		p_str_end = strstr(p_xml_body, "</EndTime>");
		memcpy(xml_end_time, p_str_begin + 9, p_str_end - p_str_begin - 9);/*保存<EndTime>到xml_end_time*/
		printf("<EndTime>:%s\r\n", xml_end_time);

		p_str_begin = strstr(p_xml_body, "<RecorderID>");/*查找字符串"<RecorderID>"*/
		p_str_end = strstr(p_xml_body, "</RecorderID>");
		memcpy(xml_recorder_id, p_str_begin + 12, p_str_end - p_str_begin - 12);/*保存<RecorderID>到xml_recorder_id*/
		printf("<RecorderID>:%s\r\n", xml_recorder_id);
		printf("***********RECORD INFO END***********\r\n");
		for (;;)
		{
			//record_list_ret = dt_eXosip_callback.dt_eXosip_getRecordTime(xml_start_time, xml_end_time, item_start_time, item_end_time);
			//if (0 > record_list_ret)
			//{
			//	break;
			//}
			//else
			{
				char temp_body[1024];
				memset(temp_body, 0, 1024);
				_snprintf_s(temp_body, 1024, "<Item>\r\n"
					"<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
					"<Name>%s</Name>\r\n"/*设备/区域名称*/
					"<FilePath>%s</FilePath>\r\n"/*文件路径名*/
					"<Address>Address1</Address>\r\n"/*录像地址*/
					"<StartTime>%s</StartTime>\r\n"/*录像开始时间*/
					"<EndTime>%s</EndTime>\r\n"/*录像结束时间*/
					"<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
					"<Type>time</Type>\r\n"/*录像产生类型*/
					"<RecorderID>%s</RecorderID>\r\n"/*录像触发者ID*/
					"</Item>\r\n",
					xml_device_id,
					device_info.device_name,
					xml_file_path,
					item_start_time,
					item_end_time,
					xml_recorder_id);
				strcat_s(rsp_item_body, sizeof(rsp_item_body), temp_body);
				record_list_num++;
				if (0 == record_list_ret)
				{
					break;
				}
			}
		}
		if (0 >= record_list_num)/*未检索到任何设备视音频文件*/
		{
			return;
		}
		_snprintf_s(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
			"<Response>\r\n"
			"<CmdType>RecordInfo</CmdType>\r\n"/*命令类型*/
			"<SN>%s</SN>\r\n"/*命令序列号*/
			"<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
			"<Name>%s</Name>\r\n"/*设备/区域名称*/
			"<SumNum>%d</SumNum>\r\n"/*查询结果总数*/
			"<RecordList Num=\"%d\">\r\n"/*文件目录项列表*/
			"%s\r\n"
			"</RecordList>\r\n"
			"</Response>\r\n",
			xml_cmd_sn,
			xml_device_id,
			device_info.device_name,
			record_list_num,
			record_list_num,
			rsp_item_body);
	}
	// 	else if (0 == strcmp(xml_cmd_type, "RecordInfo"))/*设备视音频文件检索*/
	// 	{
	// 		
	//     }
	else/*CmdType为不支持的类型*/
	{
		dt_eXosip_keepAlive(context);
		printf("**********OTHER TYPE START**********\r\n");
		printf("***********OTHER TYPE END***********\r\n");
		return;
	}
	osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
	osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
	eXosip_message_send_request(context, rsp_msg);/*回复"MESSAGE"请求*/
	printf("eXosip_message_send_request success!\r\n");
}

/*消息循环处理*/
/*EXOSIP_MESSAGE_NEW-->EXOSIP_MESSAGE_ANSWERED-->EXOSIP_CALL_INVITE-->EXOSIP_CALL_ACK-->EXOSIP_MESSAGE_ANSWERED*/
void dt_eXosip_processEvent(eXosip_t* context)
{
	eXosip_event_t* g_event = NULL;/*消息事件*/
	osip_message_t* g_answer = NULL;/*请求的确认型应答*/
	while (1)
	{

		/*等待新消息的到来*/
		g_event = eXosip_event_wait(context, 0, 200);/*侦听消息的到来*/
		eXosip_lock(context);
		eXosip_default_action(context, g_event);
		//eXosip_automatic_refresh(context);/*Refresh REGISTER and SUBSCRIBE before the expiration delay*/
		eXosip_unlock(context);
		//检查是否发心跳信息
		if (SenAliveFlag)
		{
			dt_eXosip_keepAlive(context);
			SenAliveFlag = FALSE;
		}
		if (NULL == g_event)
		{
			continue;
		}
		dt_eXosip_printEvent(context, g_event);
		/*处理感兴趣的消息*/
		switch (g_event->type)
		{
			/*即时消息：通信双方无需事先建立连接*/
		case EXOSIP_MESSAGE_NEW:/*MESSAGE:MESSAGE*/
		{
			printf("\r\n<EXOSIP_MESSAGE_NEW>\r\n");
			if (MSG_IS_MESSAGE(g_event->request))/*使用MESSAGE方法的请求*/
			{
				/*设备控制*/
				/*报警事件通知和分发：报警通知响应*/
				/*网络设备信息查询*/
				/*设备视音频文件检索*/
				printf("<MSG_IS_MESSAGE>\r\n");
				eXosip_lock(context);
				eXosip_message_build_answer(context, g_event->tid, 200, &g_answer);/*Build default Answer for request*/
				eXosip_message_send_answer(context, g_event->tid, 200, g_answer);/*按照规则回复200OK*/
				printf("eXosip_message_send_answer success!\r\n");
				eXosip_unlock(context);
				// 					if(poweron)
				// 					    dt_eXosip_keepAlive(context);
				// 					poweron = FALSE;
				dt_eXosip_paraseMsgBody(context, g_event);/*解析MESSAGE的XML消息体，同时保存全局会话ID*/
			}
		}
		break;
		/*即时消息回复的200OK*/
		case EXOSIP_MESSAGE_ANSWERED:/*200OK*/
		{
			/*设备控制*/
			/*报警事件通知和分发：报警通知*/
			/*网络设备信息查询*/
			/*设备视音频文件检索*/
			printf("\r\n<EXOSIP_MESSAGE_ANSWERED>\r\n");
		}
		break;
		/*以下类型的消息都必须事先建立连接*/
		case EXOSIP_CALL_INVITE:/*INVITE*/
		{
			printf("\r\n<EXOSIP_CALL_INVITE>\r\n");
			if (MSG_IS_INVITE(g_event->request))/*使用INVITE方法的请求*/
			{
				/*实时视音频点播*/
				/*历史视音频回放*/
				/*视音频文件下载*/
				osip_message_t* asw_msg = NULL;/*请求的确认型应答*/
				char sdp_body[4096];

				memset(sdp_body, 0, 4096);
				printf("<MSG_IS_INVITE>\r\n");

				eXosip_lock(context);
				if (0 != eXosip_call_build_answer(context, g_event->tid, 200, &asw_msg))/*Build default Answer for request*/
				{
					eXosip_call_send_answer(context, g_event->tid, 603, NULL);
					eXosip_unlock(context);
					printf("eXosip_call_build_answer error!\r\n");
					break;
				}
				eXosip_unlock(context);
				_snprintf_s(sdp_body, 4096, "v=0\r\n"/*协议版本*/
					"o=%s 0 0 IN IP4 %s\r\n"/*会话源*//*用户名/会话ID/版本/网络类型/地址类型/地址*/
					"s=(null)\r\n"/*会话名*/
					"c=IN IP4 %s\r\n"/*连接信息*//*网络类型/地址信息/多点会议的地址*/
					"t=0 0\r\n"/*时间*//*开始时间/结束时间*/
					"m=video %s RTP/AVP 96\r\n"/*媒体/端口/传送层协议/格式列表*/
					"a=sendonly\r\n"/*收发模式*/
					"a=rtpmap:96 PS/90000\r\n"/*净荷类型/编码名/时钟速率*/
					"a=username:%s\r\n"
					"a=password:%s\r\n"
					"y=0999999999\r\n"
					"f=\r\n",
					device_info.ipc_id,
					device_info.ipc_ip,
					device_info.ipc_ip,
					device_info.ipc_media_port,
					device_info.ipc_id,
					device_info.ipc_pwd);
				eXosip_lock(context);
				osip_message_set_body(asw_msg, sdp_body, strlen(sdp_body));/*设置SDP消息体*/
				osip_message_set_content_type(asw_msg, "application/sdp");
				eXosip_call_send_answer(context, g_event->tid, 200, asw_msg);/*按照规则回复200OK with SDP*/
				printf("eXosip_call_send_answer success!\r\n");
				eXosip_unlock(context);
			}
		}
		break;
		case EXOSIP_CALL_ACK:/*ACK*/
		{
			/*实时视音频点播*/
			/*历史视音频回放*/
			/*视音频文件下载*/
			printf("\r\n<EXOSIP_CALL_ACK>\r\n");/*收到ACK才表示成功建立连接*/
			dt_eXosip_paraseInviteBody(context, g_event);/*解析INVITE的SDP消息体，同时保存全局INVITE连接ID和全局会话ID*/
		}
		break;
		case EXOSIP_CALL_CLOSED:/*BEY*/
		{
			/*实时视音频点播*/
			/*历史视音频回放*/
			/*视音频文件下载*/
			printf("\r\n<EXOSIP_CALL_CLOSED>\r\n");
			if (MSG_IS_BYE(g_event->request))
			{
				printf("<MSG_IS_BYE>\r\n");
				if ((0 != g_did_realPlay) && (g_did_realPlay == g_event->did))/*实时视音频点播*/
				{
					/*关闭：实时视音频点播*/
					printf("realPlay closed success!\r\n");
					g_did_realPlay = 0;
				}
				else if ((0 != g_did_backPlay) && (g_did_backPlay == g_event->did))/*历史视音频回放*/
				{
					/*关闭：历史视音频回放*/
					printf("backPlay closed success!\r\n");
					g_did_backPlay = 0;
				}
				else if ((0 != g_did_fileDown) && (g_did_fileDown == g_event->did))/*视音频文件下载*/
				{
					/*关闭：视音频文件下载*/
					printf("fileDown closed success!\r\n");
					g_did_fileDown = 0;
				}
				if ((0 != g_call_id) && (0 == g_did_realPlay) && (0 == g_did_backPlay) && (0 == g_did_fileDown))/*设置全局INVITE连接ID*/
				{
					printf("call closed success!\r\n");
					g_call_id = 0;
				}
			}
		}
		break;
		case EXOSIP_CALL_MESSAGE_NEW:/*MESSAGE:INFO*/
		{
			/*历史视音频回放*/
			printf("\r\n<EXOSIP_CALL_MESSAGE_NEW>\r\n");
			if (MSG_IS_INFO(g_event->request))
			{
				osip_body_t* msg_body = NULL;

				printf("<MSG_IS_INFO>\r\n");
				osip_message_get_body(g_event->request, 0, &msg_body);
				if (NULL != msg_body)
				{
					eXosip_call_build_answer(context, g_event->tid, 200, &g_answer);/*Build default Answer for request*/
					eXosip_call_send_answer(context, g_event->tid, 200, g_answer);/*按照规则回复200OK*/
					printf("eXosip_call_send_answer success!\r\n");
					dt_eXosip_paraseInfoBody(context, g_event);/*解析INFO的RTSP消息体*/
				}
			}
		}
		break;
		case EXOSIP_CALL_MESSAGE_ANSWERED:/*200OK*/
		{
			/*历史视音频回放*/
			/*文件结束时发送MESSAGE(File to end)的应答*/
			printf("\r\n<EXOSIP_CALL_MESSAGE_ANSWERED>\r\n");
		}
		break;
		/*其它不感兴趣的消息*/
		default:
		{
			printf("\r\n<OTHER>\r\n");
			printf("eXosip event type:%d\n", g_event->type);
		}
		break;
		}
	}
}


/*解析INFO的RTSP消息体*/
void dt_eXosip_paraseInfoBody(eXosip_t* context, eXosip_event_t* p_event)
{
	osip_body_t* p_msg_body = NULL;
	char* p_rtsp_body = NULL;
	char* p_str_begin = NULL;
	char* p_str_end = NULL;
	char* p_strstr = NULL;
	char rtsp_scale[10];
	char rtsp_range_begin[10];
	char rtsp_range_end[10];
	char rtsp_pause_time[10];

	memset(rtsp_scale, 0, 10);
	memset(rtsp_range_begin, 0, 10);
	memset(rtsp_range_end, 0, 10);
	memset(rtsp_pause_time, 0, 10);

	osip_message_get_body(p_event->request, 0, &p_msg_body);
	if (NULL == p_msg_body)
	{
		printf("osip_message_get_body null!\r\n");
		return;
	}
	p_rtsp_body = p_msg_body->body;
	printf("osip_message_get_body success!\r\n");

	p_strstr = strstr(p_rtsp_body, "PLAY");
	if (NULL != p_strstr)/*查找到字符串"PLAY"*/
	{
		/*播放速度*/
		p_str_begin = strstr(p_rtsp_body, "Scale:");/*查找字符串"Scale:"*/
		p_str_end = strstr(p_rtsp_body, "Range:");
		memcpy(rtsp_scale, p_str_begin + 6, p_str_end - p_str_begin - 6);/*保存Scale到rtsp_scale*/
		printf("PlayScale:%s\r\n", rtsp_scale);
		/*播放范围*/
		p_str_begin = strstr(p_rtsp_body, "npt=");/*查找字符串"npt="*/
		p_str_end = strstr(p_rtsp_body, "-");
		memcpy(rtsp_range_begin, p_str_begin + 4, p_str_end - p_str_begin - 4);/*保存RangeBegin到rtsp_range_begin*/
		printf("PlayRangeBegin:%s\r\n", rtsp_range_begin);
		p_str_begin = strstr(p_rtsp_body, "-");/*查找字符串"-"*/
		strcpy_s(rtsp_range_end, sizeof(rtsp_range_end), p_str_begin + 1);/*保存RangeEnd到rtsp_range_end*/
		printf("PlayRangeEnd:%s\r\n", rtsp_range_end);
		//dt_eXosip_callback.dt_eXosip_playControl("PLAY", rtsp_scale, NULL, rtsp_range_begin, rtsp_range_end);
		return;
	}

	p_strstr = strstr(p_rtsp_body, "PAUSE");
	if (NULL != p_strstr)/*查找到字符串"PAUSE"*/
	{
		/*暂停时间*/
		p_str_begin = strstr(p_rtsp_body, "PauseTime:");/*查找字符串"PauseTime:"*/
		strcpy_s(rtsp_pause_time, sizeof(rtsp_pause_time), p_str_begin + 10);/*保存PauseTime到rtsp_pause_time*/
		printf("PauseTime:%3s\r\n", rtsp_pause_time);
		//dt_eXosip_callback.dt_eXosip_playControl("PAUSE", NULL, rtsp_pause_time, NULL, NULL);
		return;
	}

	printf("can`t find string PLAY or PAUSE!");
}


char eXosip_server_id[30] = "37010200492000000001"; //123 "13110610025571659000";:121"13101811171909469000";
char eXosip_server_ip[20] = "192.168.0.191";
char eXosip_server_port[10] = "60400";
//char eXosip_ipc_id[30]              = "001200000410000";
char eXosip_ipc_id[30] = "34020000001320000001";
char eXosip_ipc_pwd[20] = "admin123";
char eXosip_ipc_ip[20] = "192.168.0.191";
char eXosip_ipc_media_port[10] = "20000";
char eXosip_ipc_sess_port[10] = "5080";

//char eXosip_alarm_id[30]            = "001200000410010"; //"34020000001340000005";
char eXosip_alarm_id[30] = "34020000001340000010";


char eXosip_media_ip[30] = "10.0.0.99";
char eXosip_media_port[10] = "6000";

char eXosip_device_name[30] = "zwj-ceshi";
char eXosip_device_manufacturer[30] = "datang";
char eXosip_device_model[30] = "ABC_model2";
char eXosip_device_firmware[30] = "V1.0";
char eXosip_device_encode[10] = "ON";
char eXosip_device_record[10] = "OFF";

char eXosip_status_on[10] = "ON";
char eXosip_status_ok[10] = "OK";
char eXosip_status_online[10] = "ONLINE";
char eXosip_status_guard[10] = "OFFDUTY";
char eXosip_status_time[30] = "2014-01-17T16:30:20";
const char* sip_server_ip = "192.168.0.191";
uint32_t sip_local_port = 60433;
uint32_t sip_server_port = 60433;
void init_param()
{





	device_info.server_id = eXosip_server_id;
	device_info.server_ip = eXosip_server_ip;
	device_info.server_port = eXosip_server_port;
	device_info.ipc_id = eXosip_ipc_id;
	device_info.ipc_pwd = eXosip_ipc_pwd;
	device_info.ipc_ip = eXosip_ipc_ip;
	device_info.ipc_media_port = eXosip_ipc_media_port;
	device_info.ipc_sess_port = eXosip_ipc_sess_port;

	device_info.alarm_id = eXosip_alarm_id;

	device_info.media_ip = eXosip_media_ip;
	device_info.media_port = eXosip_media_port;

	device_info.device_name = eXosip_device_name;
	device_info.device_manufacturer = eXosip_device_manufacturer;
	device_info.device_model = eXosip_device_model;
	device_info.device_firmware = eXosip_device_firmware;
	device_info.device_encode = eXosip_device_encode;
	device_info.device_record = eXosip_device_record;

	device_status.status_on = eXosip_status_on;
	device_status.status_ok = eXosip_status_ok;
	device_status.status_online = eXosip_status_online;
	device_status.status_guard = eXosip_status_guard;
	device_status.status_time = eXosip_status_time;
}

int main(int argc, char* argv[])
{



	init_param();


	eXosip_initial();


	// 配置exosip库参数，如IP地址和端口
	int  ret_code = eXosip_listen_addr(g_context_sip_ptr, IPPROTO_UDP, NULL, atoi(device_info.ipc_sess_port), AF_INET, 0);
	if (ret_code != OSIP_SUCCESS)
	{
		printf("eXosip_listen_addr error!\n");
		// eXosip_quit(exosip_);
		 //exit(1);
	}
	dt_eXosip_register(g_context_sip_ptr, 3600);
	dt_eXosip_processEvent(g_context_sip_ptr);
}