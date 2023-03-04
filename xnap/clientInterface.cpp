//
// This file is developed by Philip Astillo as extension for
// UERANSIM open source project.
// An attempt for the handover procedure.
//

#include "task.hpp"
#include "utils.hpp"

#include <algorithm>

//#include <utils/common.hpp>

#include <gnb/app/task.hpp>
#include <gnb/rrc/task.hpp>
#include <gnb/sctp/task.hpp>

#include <asn/xnap/BroadcastPLMNinTAISupport-Item.h>
#include <asn/xnap/InitiatingMessage.h>
#include <asn/xnap/GlobalAMF-Set-Information.h>
#include <asn/xnap/GlobalgNB-ID.h>
#include <asn/xnap/GlobalNG-RANNode-ID.h>
#include <asn/xnap/ProtocolIE-Field.h>
#include <asn/xnap/S-NSSAI.h>
#include <asn/xnap/SliceSupport-List.h>
#include <asn/xnap/TAISupport-Item.h>
#include <asn/xnap/XnSetupRequest.h>

//=====================Added by Group N==================//
#include <asn/xnap/ServedCellInformation-NR.h>
#include <asn/xnap/ServedCells-NR.h>
#include <asn/xnap/ServedCells-NR-Item.h>
#include <asn/xnap/NR-CGI.h>
#include <asn/xnap/RANAC.h>
#include <asn/xnap/NRModeInfo.h>
#include <asn/xnap/NRModeInfoFDD.h>
#include <asn/xnap/NRModeInfoTDD.h>
#include <asn/xnap/Connectivity-Support.h>
#include <asn/xnap/BroadcastPLMNs.h>
#include <asn/xnap/PLMN-Identity.h>
#include <asn/xnap/XnSetupResponse.h>
#include <asn/xnap/SuccessfulOutcome.h>
#include <asn/xnap/XnAP-PDU.h>
//=====================Group N===========================//

namespace nr::gnb
{
	
	void XnapTask::createGnbContext(const GnbAssociatedConfig &conf)
	{
		auto *ctx = new XnapGnbContext();
		ctx->ctxId = utils::NextId();
		ctx->address = conf.address;
		ctx->port = conf.port;
		m_gnbCtx[ctx->ctxId] = ctx;
	}
	
	void XnapTask::xnSctpConnect()
	{
		for (auto &gnbCtx : m_gnbCtx)
		{
			auto *msg1 = new NmGnbSctp(NmGnbSctp::XN_SCTP_CONNECT);
			msg1->clientId = gnbCtx.second->ctxId;
			msg1->remoteAddress = gnbCtx.second->address;
			msg1->remotePort = gnbCtx.second->port;
			msg1->ppid = sctp::PayloadProtocolId::XNAP;
			msg1->localAddress = m_base->config->xnapIp;
			msg1->localPort = 0;
			msg1->associatedTask = this;
			msg1->nodeType = "client";
			m_base->sctpXnapTask->push(msg1);
		}
	}
	
	void XnapTask::xnSetupRequestTransmit()
	{
		// construct the XN_SETUP_REQUEST message here then send via m_base->sctpXnapTask object
		
		//TODO: This part prepares the GlobalNG-RANNode ID IEs
		auto *globalGnbId = asn::New<GlobalgNB_ID>();
		globalGnbId->gnb_id.present = GNB_ID_Choice_PR_gnb_ID;
		asn::SetBitString(globalGnbId->gnb_id.choice.gnb_ID, octet4{m_base->config->getGnbId()});
		asn::SetOctetString3(globalGnbId->plmn_id, xnap_utils::PlmnToOctet3(m_base->config->plmn));
			
		auto *globalNGRAN = asn::New<GlobalNG_RANNode_ID>();
		globalNGRAN->present = GlobalNG_RANNode_ID_PR_gNB;
		globalNGRAN->choice.gNB = globalGnbId;
			
		auto *ieGlobalNGRAN = asn::New<XnSetupRequest_IEs>();
		ieGlobalNGRAN->id = XNAP_Protocol_IE_id_GlobalNG_RAN_node_ID;
		ieGlobalNGRAN->criticality = Criticality_reject;
		ieGlobalNGRAN->value.present = XnapSetupRequestIEs__value_PR_GlobalRANNodeID;
		ieGlobalNGRAN->value.choice.GlobalNgRANNodeID = *globalNGRAN;

				
		// TODO: This part prepares the TAISupport-List
		auto *broadcastPLMN = asn::New<BroadcastPLMNinTAISupport_Item>();
		asn::SetOctetString3(broadcastPLMN->plmn_id, xnap_utils::PlmnToOctet3(m_base->config->plmn));
		for (auto &nss : m_base->config->nssai.slices)
		{
			auto *nssai = asn::New<S_NSSAI>();
			asn::SetOctetString1(nssai->sst, static_cast<uint8_t>(nss.sst));
			if(nss.sd.has_value())
			{
				asn::SetOctetString3(nssai->sd, octet3{nss.sd.value()});
			}
			asn::SequenceAdd(broadcastPLMN->tAISliceSupport_List,  nssai);
		}
			
		auto *supportedTAI = asn::New<TAISupport_Item>();
		asn::SetOctetString3(supportedTAI->tac, octet3{m_base->config->tac});
		asn::SequenceAdd(supportedTAI->broadcastPLMNS_list, broadcastPLMN);
			
		auto *ieTAISupportList  = asn::New<XnSetupRequest_IEs>();
		ieTAISupportList->id = XNAP_Protocol_IE_id_TAISupport_list;
		ieTAISupportList->criticality = Criticality_reject;
		ieTAISupportList->value.present = XnapSetupRequestIEs__value_PR_TAISupportList,
		asn::SequenceAdd(ieTAISupportList->value.choice.TAISupportList , supportedTAI);

		
		// TODO: This part prepares the AMF-SET-INFORMATION
		auto *ieAmfSetInfo = asn::New<XnSetupRequest_IEs>();
		ieAmfSetInfo->id = XNAP_Protocol_IE_id_AMF_Set_Information;
		ieAmfSetInfo->criticality = Criticality_reject;
		ieAmfSetInfo->value.present = XnapSetupRequestIEs__value_PR_AMFSetInformation;
			
		for(auto it = m_amfCtx.begin(); it != m_amfCtx.end(); ++it)
		{
			auto servedGuamiList = it->second->servedGuamiList;
			for(auto i = servedGuamiList.begin(); i != servedGuamiList.end(); ++i)
			{
				auto *globalAMFSetInfo = asn::New<GlobalAMF_Set_Information>();
				asn::SetOctetString3(globalAMFSetInfo->plmn_ID,  xnap_utils::PlmnToOctet3((*i)->guami.plmn));
				asn::SetBitStringInt<8>((*i)->guami.amfRegionId,  globalAMFSetInfo->amf_region_id);
				asn::SetBitStringInt<10>((*i)->guami.amfSetId, globalAMFSetInfo->amf_set_id);
			
				asn::SequenceAdd(ieAmfSetInfo->value.choice.AMFSetInfo, globalAMFSetInfo);
			}
		}
		
	
		//=======================Added by Group N=======================================================================//
		//TODO: This part prepares the List of Served Cells NR
		auto *ieServedCellsNR = asn::New<XnSetupRequest_IEs>();
		ieServedCellsNR->id = XNAP_Protocol_IE_id_ServedCells_NR;
		ieServedCellsNR->criticality = Criticality_reject;
		ieServedCellsNR->value.present = XnapSetupRequestIEs__value_PR_ServedCellsNR;

		auto *servedCellsNR = asn::New<ServedCellInformation_NR>();
		
		//NR-PCI		
		servedCellsNR->nrPCI = m_base->config->nrpci; //NR-PCI , from config file
			
		//NR CGI
		auto *nrCGI = asn::New<NR_CGI>();
		asn::SetOctetString3(nrCGI->plmn_id, xnap_utils::PlmnToOctet3(m_base->config->plmn)); //plmn identity
		asn::SetBitStringLong<36>(m_base->config->nci, nrCGI->nr_CI); //NR Cell Identity BIT STRING (SIZE(36))	
		servedCellsNR->cellID = *nrCGI;
		
		//TAC
		asn::SetOctetString3(servedCellsNR->tac, octet3{m_base->config->tac}); //TAC, octet string size 3, from config file
			
		//RANAC *optional*	
		asn::SetOctetString3(servedCellsNR->ranac, octet3{m_base->config->ranac}); //RANAC, octet string size 3, from config file
		
		//broadcast PLMN
		
		auto *broadcastPLMNnr = asn::New<BroadcastPLMNs>();
	    asn::SetOctetString3(broadcastPLMNnr->plmn_ID1, xnap_utils::PlmnToOctet3(m_base->config->plmn)); //plmn identity
		servedCellsNR->broadcastPLMN = *broadcastPLMNnr;
		
		//NR-Mode Info	
		//FDD
		//uplink NR frequency
		servedCellsNR->nrModeInfo.choice.fdd.ulNRFrequencyInfo.nrARFCN = 1920;//hard coded
		servedCellsNR->nrModeInfo.choice.fdd.ulNRFrequencyInfo.frequencyBand_List.FB1.nr_frequency_band = 2100;//hard coded
		servedCellsNR->nrModeInfo.choice.fdd.ulNRFrequencyInfo.frequencyBand_List.FB1.supp_SUL_Band_List.SUL1.sulBandItem = 3000;//hard coded

		//downlink NR frequency
		servedCellsNR->nrModeInfo.choice.fdd.dlNRFrequencyInfo.nrARFCN = 2110;//hard coded, NRARFCN
		servedCellsNR->nrModeInfo.choice.fdd.dlNRFrequencyInfo.frequencyBand_List.FB2.nr_frequency_band = 2600;//hard coded, NR Frequency Band
		servedCellsNR->nrModeInfo.choice.fdd.ulNRFrequencyInfo.frequencyBand_List.FB2.supp_SUL_Band_List.SUL2.sulBandItem = 832;//hard coded, supported SUL
	
		//=========SUL information is not added because it is optional but may be added in the future=================//

		//uplink Transmission bandwidth
		NRSCS nrscs1 = NRSCS_scs15;//hard coded
		NRNRB nrnrb1 = NRNRB_nrb18;//hard coded
		servedCellsNR->nrModeInfo.choice.fdd.ulNRTransmissonBandwidth.nRSCS = nrscs1;//hard coded
        servedCellsNR->nrModeInfo.choice.fdd.ulNRTransmissonBandwidth.nRNRB = nrnrb1;//hard coded
        
        //downlink Transmission bandwidth
        NRSCS nrscs2 = NRSCS_scs30;//hard coded
		NRNRB nrnrb2 = NRNRB_nrb24;//hard coded
		servedCellsNR->nrModeInfo.choice.fdd.dlNRTransmissonBandwidth.nRSCS = nrscs2;//hard coded
        servedCellsNR->nrModeInfo.choice.fdd.dlNRTransmissonBandwidth.nRNRB = nrnrb2;//hard coded
		
		//TDD
		//Frequency Info
		servedCellsNR->nrModeInfo.choice.tdd.nrFrequencyInfo.nrARFCN = 2170;//hard coded
		servedCellsNR->nrModeInfo.choice.tdd.nrFrequencyInfo.frequencyBand_List.FB3.nr_frequency_band = 1800;//hard coded
		servedCellsNR->nrModeInfo.choice.tdd.nrFrequencyInfo.frequencyBand_List.FB3.supp_SUL_Band_List.SUL3.sulBandItem = 900;//hard coded
		
		//Transmission bandwidth
		NRSCS nrscs3 = NRSCS_scs60;//hard coded
		NRNRB nrnrb3 = NRNRB_nrb25;//hard coded
		servedCellsNR->nrModeInfo.choice.tdd.nrTransmissonBandwidth.nRSCS = nrscs3;//hard coded
		servedCellsNR->nrModeInfo.choice.tdd.nrTransmissonBandwidth.nRNRB = nrnrb3;//hard coded
		
		//Measurement Timing Configuration octet string
		asn::SetOctetString4(servedCellsNR->measurementTimingConfiguration, octet4{m_base->config->measurementTimingConfiguration});
			
		//Connectivity Support
		eNDC_Support supported = eNDC_Support_supported;
		servedCellsNR->connectivitySupport.eNDC_Support = supported;
		
		auto *servedCellsNRItem = asn::New<ServedCells_NR_Item>();
		
		servedCellsNRItem->served_cell_info_NR = *servedCellsNR;
		asn::SequenceAdd(ieServedCellsNR->value.choice.ServedCellNR, servedCellsNRItem);
		
		//Neighbour Information NR is optional
				
		auto *pdu = asn::xnap::NewMessagePdu<XnSetupRequest>(
								{ieGlobalNGRAN, ieTAISupportList, ieAmfSetInfo, ieServedCellsNR}); 
		
		//===========================================Group N============================================//*/
    	
	}
		
}// namespace nr::gnb
