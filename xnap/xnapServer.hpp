////
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//
#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>


#include <gnb/types.hpp>
#include <gnb/nts.hpp>

#include <utils/nts.hpp>
#include <lib/sctp/sctp.hpp>

namespace nr::gnb
{
	
class XnapServer : public NtsTask
{
	private:
		std::unique_ptr<Logger> m_logger;
		sctp::SctpServer *m_xnapServer;
		NtsTask *m_mainTask;
		
		std::string localAddress;
		
		struct ClientEntry
		{
			int socketId;
			sctp::SctpClient *client;
			
			ScopedThread *receiverThread;
			sctp::ISctpHandler *handler;
			NtsTask *associatedTask;
		};
		std::unordered_map<int, ClientEntry *> m_clients;
		
	
	public:
		explicit XnapServer(TaskBase *base);
		~XnapServer() override = default;
		
		
	protected:
		void onStart() override;
		void onLoop() override;
		void onQuit() override;
		
	public:
		void initialize(NtsTask *mainTask);
	
};
	
}// namespace nr::gnb
