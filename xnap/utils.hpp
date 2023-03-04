//
// This file is a part of UERANSIM open source project.
// is added by Philip Virgil Astillo
//
//

#pragma once

#include <gnb/types.hpp>
#include <lib/asn/xnap.hpp>
#include <lib/asn/utils.hpp>
#include <utils/common.hpp>
#include <utils/common_types.hpp>

namespace nr::gnb::xnap_utils
{
	
	octet3 PlmnToOctet3(const Plmn &plmn);
	
	
}
