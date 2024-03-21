
#ifndef _C_SIP_EVENT_H_
#define _C_SIP_EVENT_H_
#include <mutex>

#include <queue>
#include <functional>
#include <memory>
#include "eXosip2/eXosip.h"
#include "csip_msg_dispatch.h"
namespace chen {

    struct csip_event_t
    {
        int                 value;
        eXosip_event_type_t     sip_msg_id;
      //  const char* name;
      //  handler_sip_type          proc;
        struct eXosip_t* excontext;
        eXosip_event_t* exevent;
        uint64_t            id;

        csip_event_t() /*= default*/ {}
    };
   // typedef std::function<int32_t (const csip_event_t*&)> event_proc;

}

#endif // _C_SIP_EVENT_H_
