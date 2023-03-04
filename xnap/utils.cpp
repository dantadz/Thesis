//
// This file is a part of UERANSIM open source project.
// is added by Philip Virgil Astillo
//
//

#include "utils.hpp"

namespace nr::gnb::xnap_utils
{
	octet3 PlmnToOctet3(const Plmn &plmn)
	{
		int mcc = plmn.mcc;
		int mcc3 = mcc % 10;
		int mcc2 = (mcc % 100) / 10;
		int mcc1 = (mcc % 1000) / 100;

		int mnc = plmn.mnc;

		if (plmn.isLongMnc)
		{
			int mnc1 = mnc % 1000 / 100;
			int mnc2 = mnc % 100 / 10;
			int mnc3 = mnc % 10;

			int octet1 = mcc2 << 4 | mcc1;
			int octet2 = mnc1 << 4 | mcc3;
			int octet3 = mnc3 << 4 | mnc2;

			return {(uint8_t)octet1, (uint8_t)octet2, (uint8_t)octet3};
		}
		else
		{
			int mnc1 = mnc % 100 / 10;
			int mnc2 = mnc % 10;
			int mnc3 = 0xF;

			int octet1 = mcc2 << 4 | mcc1;
			int octet2 = mnc3 << 4 | mcc3;
			int octet3 = mnc2 << 4 | mnc1;

			return {(uint8_t)octet1, (uint8_t)octet2, (uint8_t)octet3};
		}
	}
}