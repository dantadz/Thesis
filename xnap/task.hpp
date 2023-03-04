//
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//

#pragma once

#include "xnapServer.hpp"

//#include <memory>
//#include <thread>
#include <optional>
#include <unordered_map>
#include <map>
#include <vector>

#include <gnb/app/task.hpp>
#include <gnb/ngap/task.hpp>
#include <gnb/sctp/task.hpp>

#include <gnb/nts.hpp>
#include <gnb/types.hpp>
#include <lib/app/monitor.hpp>
#include <utils/logger.hpp>
#include <utils/nts.hpp>


extern "C"
{
	struct XnAP_PDU;
	struct XnSetupRequest;
	struct XnSetupResponse;
	struct XnSetupFailure;
}


namespace nr::gnb
{
	class SctpTask;
	class NgapTask;
	class GnbAppTask;
	
	class XnapTask : public NtsTask
	{
		private:
			TaskBase *m_base;
			std::unique_ptr<Logger> m_logger;
			std::unordered_map<int, XnapGnbContext *> m_gnbCtx;
			
			XnapServer *m_xnapServer;
			SctpTask *m_sctpServer;

			uint32_t m_oldXnapUeCntr;
			uint32_t m_newXnapUeCntr;
			std::unordered_map<int, XnapUeContext *> m_xnapUeCtx;
			
			std::unordered_map<int, NgapAmfContext *> m_amfCtx;
			
		public:
			explicit XnapTask(TaskBase *base);
			~XnapTask() override = default;
			
			void assignGuami(ServedGuami servedGuami);
			
		protected:
			void onStart() override;
			void onLoop() override;
			void onQuit() override;
			
		private:
		
		/* Client Interface Management */
		void createGnbContext(const GnbAssociatedConfig &config);
		void xnSctpConnect();
		void xnSetupRequestTransmit();
		
		/* Server Interface Management */
		void registerAcceptedClient(int socketId);
			
			
		/* Common Interface Magement */
		void xnHandoverRequestTransmit(uint16_t ueId);
	};
	
} // namespace nr::gnb
