//
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//

//Group N
#include <gnb/sctp/task.hpp>

#include "task.hpp"

#include <sstream>
#include <cstring>
#include <thread>
#include <chrono>
#include <utility>

namespace nr::gnb
{
	
//======================================================================//

	XnapTask::XnapTask(TaskBase *base) : m_base{base}
	{
		m_logger = base->logBase->makeUniqueLogger("xnap-proc");
		
		m_xnapServer = new XnapServer(base);	
		m_xnapServer->initialize(this);
		
		m_sctpServer = new SctpTask(base->logBase);
		
		m_oldXnapUeCntr = 0;
		m_newXnapUeCntr = 0;
	}
	
	void XnapTask::onStart()
	{
		m_sctpServer->start();
		m_xnapServer->start();	
		
		for(auto &gnbAssocConfig : m_base->config->gnbAssocConfigs)
			createGnbContext(gnbAssocConfig);
		if(m_gnbCtx.empty())
			m_logger->warn("No Associated gNB configuration is provided");
	
		//xnSctpConnect();
	}
	
	void XnapTask::onLoop()
	{
		NtsMessage *msg = take();
		if (!msg)
			return;
		
		switch(msg->msgType)
		{
			case NtsMessageType::GNB_SCTP:
			{
				auto *w = dynamic_cast<NmGnbSctp*>(msg);
				switch(w->present)
				{
					case NmGnbSctp::ASSOCIATION_SETUP:
						//m_logger->debug("Association setup of [%s] to [%s] successful", w->nodeType.c_str(), w->remoteAddress.c_str());
						if(w->nodeType == "client"){
							
							m_logger->debug("Sending Xn Setup Request....");
							xnSetupRequestTransmit();
							auto *msg2 = new NmGnbSctp(NmGnbSctp::XN_SETUP_RESPONSE);
							m_base->sctpXnapTask->push(msg2);
						}
						break;
					case NmGnbSctp::ASSOCIATION_SHUTDOWN:
						m_logger->debug("Association shutdown of [%s] to [%s] successful", w->nodeType.c_str(), w->remoteAddress.c_str());
						break;
					default:
						m_logger->unhandledNts(msg);
				}
				break;
			}
			case NtsMessageType::GNB_NGAP_TO_XNAP:
			{
				
				auto *w = dynamic_cast<NmGnbNgapToXnap*>(msg);
				switch(w->present)
				{
					case NmGnbNgapToXnap::XNAP_GNB_CONNECT:
					m_amfCtx = w->amfCtx;
					xnSctpConnect();	// Initiate SCTP connection with target gNodes.
					break;
				}
				break;
			}
			case NtsMessageType::XNAP_CLIENT:
			{
				auto *w = dynamic_cast<NmGnbXnapClientCon*>(msg);
				switch(w->present)
				{
					case NmGnbXnapClientCon::XNAP_CLIENT_CON:
							registerAcceptedClient(w->socketId);	// See gnb/xnap/interface.cpp
						break;
					case NmGnbXnapClientCon::XNAP_CLIENT_ENTRY:
							m_logger->debug("Received a message...");
							m_logger->debug("Sending Xn Setup Respond....");
							xnSetupRequestTransmit();
						break;
					case NmGnbXnapClientCon::XN_HANDOVER:
							m_logger->debug("Performing Handover....");
						break;
					default:
						m_logger->unhandledNts(msg);
						break;
				}				
				break;
			}
			default:
				m_logger->unhandledNts(msg);
				break;
		}
		delete msg;
	}
	
	void XnapTask::onQuit()
	{
		for (auto &i : m_gnbCtx)
			delete i.second;
		
		m_gnbCtx.clear();
	}
	

}// namespace::gnb
