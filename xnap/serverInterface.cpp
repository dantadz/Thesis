
#include "task.hpp"

#include <gnb/app/task.hpp>
#include <gnb/sctp/task.hpp>
#include <utils/common.hpp>

namespace nr::gnb
{
	void XnapTask::registerAcceptedClient(int socketId)
	{
		
		m_logger->info("NG-RAN is being registered to client-info-list (server)");
		
		auto *msg = new NmGnbXnapClientCon(NmGnbXnapClientCon::XNAP_CLIENT_ENTRY);
		msg->socketId = socketId;
		msg->associatedTask = this;
		msg->ppid = sctp::PayloadProtocolId::XNAP;
		msg->nodeType = "server";
		m_sctpServer->push(msg);
		
		auto *msg2 = new NmGnbXnapClientCon(NmGnbXnapClientCon::XNAP_CLIENT_ENTRY);
		m_base->xnapTask->push(msg2);
	}	
}// namespace nr::gnb
