////
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//

#include "xnapServer.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <set>

#include <gnb/nts.hpp>
#include <utils/common.hpp>
#include <utils/constants.hpp>
#include <utils/libc_error.hpp>

namespace nr::gnb
{

XnapServer::XnapServer(TaskBase *base)
{
	m_logger = base->logBase->makeUniqueLogger("xnap-serv");
	try
	{
		m_xnapServer = new sctp::SctpServer(base->config->xnapIp, (uint16_t)sctp::ComPort::XNAP);
		localAddress = base->config->xnapIp;
	}
	catch(const LibError &e)
	{
		m_logger->err("SCTP-XNAP failure [%s]", e.what());
		quit();
		return;
	}
}

void XnapServer::initialize(NtsTask *mainTask)
{
	m_mainTask = mainTask;
}

void XnapServer::onStart()
{
	try
	{
		m_xnapServer->bind(localAddress, (uint16_t)sctp::ComPort::XNAP);
	}
	catch(const sctp::SctpError &exec)
	{
		m_logger->err("Binding xnap server to [%s:%d failed]. %s", localAddress.c_str(), (uint16_t)sctp::ComPort::XNAP, exec.what());
		delete m_xnapServer;
		return;
	}
	
	try
	{
		m_logger->info("SCTP-XNAP Server listening [%s:%d]", localAddress.c_str(), (uint16_t)sctp::ComPort::XNAP);
		m_xnapServer->listen();
	}
	catch (const sctp::SctpError &exec)
	{
		m_logger->err("Listening xnap server to %s:%d failed. %s", localAddress.c_str(), (uint16_t)sctp::ComPort::XNAP, exec.what());
		delete m_xnapServer;
		return;
	}
}

void XnapServer::onLoop()
{
	try
	{
		std::string clientIp="";
		int clientId=-1;
		uint16_t clientPort = 0;
		m_xnapServer->accept(clientIp, clientId, clientPort);
		
		//m_logger->info("SCTP-XNAP server accepted client [%s:%d]", clientIp.c_str(), clientPort);
		
		auto *msg = new NmGnbXnapClientCon(NmGnbXnapClientCon::XNAP_CLIENT_CON);
		msg->socketId = clientId;
		m_mainTask->push(msg);
		
	}
	catch(const sctp::SctpError &exec)
	{
		m_logger->err("Accepting xnap clients to %s:%d failed. %s", localAddress.c_str(), (uint16_t)sctp::ComPort::XNAP, exec.what());
		delete m_xnapServer;
		return;
	}
}

void XnapServer::onQuit()
{
	
}
	
	
}// namespace nr::gnb