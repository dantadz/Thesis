//
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//
//#include <utils/common.hpp>

#include "task.hpp"
#include "utils.hpp"

#include <utils/common.hpp>
#include <gnb/sctp/task.hpp>
#include <asn/xnap/NG-RANnodeUEXnAPID.h>
#include <asn/xnap/HandoverRequest.h>
#include <asn/xnap/Cause.h>
#include <asn/xnap/Target-CGI.h>
//#include "cmd_handler.cpp"
//Pradnya
#include <iostream>
#include <string> 
#define PAUSE_CONFIRM_TIMEOUT 3000
#define PAUSE_POLLING 10
//Group N
#include <asn/xnap/BroadcastPLMNinTAISupport-Item.h>
#include <asn/xnap/BroadcastPLMNinTAISupport-Item.h>
#include <gnb/xnap/clientInterface.cpp>
#include <asn/xnap/GlobalAMF-Set-Information.h>
#include <asn/xnap/GlobalgNB-ID.h>
#include <asn/xnap/GlobalNG-RANNode-ID.h>
#include <asn/xnap/ServedCellInformation-NR.h>
#include <asn/xnap/ServedCells-NR.h>
#include <asn/xnap/ServedCells-NR-Item.h>
#include <asn/xnap/NR-CGI.h>
#include <asn/xnap/RANAC.h>
#include <asn/xnap/NRModeInfoFDD.h>
#include <asn/xnap/NRModeInfoTDD.h>
#include <asn/xnap/Connectivity-Support.h>
#include <gnb/app/task.hpp>
#include <gnb/gtp/task.hpp>
#include <gnb/ngap/task.hpp>
#include <gnb/rls/task.hpp>
#include <gnb/rrc/task.hpp>
#include <gnb/sctp/task.hpp>
#include <utils/printer.hpp>

#include <asn/xnap/Cause.h>
#include <asn/xnap/CauseRadioNetworkLayer.h>
#include <asn/xnap/CauseTransportLayer.h>
#include <asn/xnap/CauseProtocol.h>
#include <asn/xnap/CauseMisc.h>
#include <asn/xnap/PLMN-Identity.h>
#include <asn/xnap/NR-Cell-Identity.h>
#include <asn/xnap/E-UTRA-CoordinationAssistanceInfo.h>
#include <asn/xnap/TransportLayerAddress.h>
#include <asn/xnap/AMF-UE-NGAP-ID.h>
#include <asn/xnap/UESecurityCapabilities.h>
#include <asn/xnap/AS-SecurityInformation.h>
#include <asn/xnap/UEAggregateMaximumBitRate.h>
#include <asn/xnap/PDUSessionResourcesToBeSetup-List.h>
#include <asn/xnap/LocationReportingInformation.h>
#include <asn/xnap/TraceActivation.h>
#include <asn/xnap/MaskedIMEISV.h>
#include <asn/xnap/UEHistoryInformation.h>
#include <asn/xnap/CPTransportLayerInformation.h>
#include <asn/xnap/GTP-TEID.h>
#include <asn/xnap/Target-CGI.h>
#include <asn/xnap/GUAMI.h>
#include <asn/xnap/PDUSession-ID.h>
#include <asn/xnap/S-NSSAI.h>
#include <asn/xnap/SliceSupport-List.h>



namespace nr::gnb
{

	
	void XnapTask::xnHandoverRequestTransmit(uint16_t ueId)
	{
		// TODO: this part prepares the NG-RANnodeUEXNAPID
		
		//Missing Source NG-RAN node UE XnAP ID Reference
		
		//Cause
		auto *C =  asn::New<Cause>();
		C->present = Cause_PR_radioNetwork;
		C->choice.radioNetwork = CauseRadioNetworkLayer_handover_desirable_for_radio_reasons;

		
		//Target Cell Global ID
		auto *targetGbl_ID = asn::New<Target_CGI>();
		targetGbl_ID->present = Target_CGI_PR_nr;
		asn::SetOctetString3(targetGbl_ID->choice.nr.plmn_id, xnap_utils::PlmnToOctet3(m_base->config->plmn));
		asn::SetBitStringInt<32>(m_base->config->nci, targetGbl_ID->choice.nr.nr_CI);
		
		/*//GUAMI	
		auto *guami = asn::New<GUAMI>();
		asn::SetOctetString3(guami->plmn_ID, xnap_utils::PlmnToOctet3(m_base->config->plmn));
		
		asn::SetBitStringInt<8>(guami->amf_region_id);
		asn::SetBitStringInt<10>(guami->amf_set_id);
		asn::SetBitStringInt<6>(guami->amf_pointer ); need work 
		
		//AMF UE NGAP ID
		auto *ngapId = asn::New<INTEGER_t>();
		ngapId -> AMF_UE_NGAP_ID_t = 
		
		//CP Transport Layer Information
		auto *endpointIp = asn::New< CPTransportLayerInformation>();
		endpointIp -> present = CPTransportLayerInformation_PR_endpointIPAddress;
		endpointIp ->choice.endpointIPAddress = TransportLayerAddress_t();	 
		
		//UE Security Capabilities
		auto *UEsec = asn::New< UESecurityCapabilities>();
		nr_EncyptionAlgorithms EA = nr_EncyptionAlgorithms_nea1_128;
		asn::SetBitStringInt<16>(EA, UEsec->nr_EncyptionAlgorithms);
		nr_IntegrityProtectionAlgorithms IPA = nr_IntegrityProtectionAlgorithms_nia1_128;
		asn::SetBitStringInt<16>(IPA, UEsec->nr_IntegrityProtectionAlgorithms);
		
		//AS Security Information
		auto *ASsec = asn::New<AS_SecurityInformation>();
		ASsec ->key_NG_RAN_Star = 1;
		//asn::SetBitString(,ASsec->key_NG_RAN_Star);
		
		//Index to RAT/Frequency Selection Priority
		
		
		//UE Aggregate Maximum Bit Rate
		auto *UEAggMaxBit = asn::New<UEAggregateMaximumBitRate>();

		//asn::SetBitStringInt(EA, UEAggMaxBit->dl_UE_AMBR);
		//UEAggMaxBit -> dl_UE_AMBR = BIT_STRING_t();
		//UEAggMaxBit -> ul_UE_AMBR = BIT_STRING_t();
		
		//PDU Session Resource To Be Setup List
		auto *PDUSessionList = asn::New<PDUSessionResourcesToBeSetup_List>();
		//for( auto )
		//PDUSession -> list = PDUSessionResourcesToBeSetup_Item();
		//auto *PDUSessionID = asn::New<PDUSession-ID>();
		
		for (auto &nssai : m_base->config->nssai.slices)
		{
			auto *item = asn::New<S_NSSAI>();
			asn::SetOctetString1(item->sst, static_cast<uint8_t>(nssai.sst));
			if (nssai.sd.has_value())
			{
				asn::SetOctetString3(item->sst, octet3{nssai.sd.value()});
			}
		}*/
		}
		
}// namespace nr::gnb10.3
