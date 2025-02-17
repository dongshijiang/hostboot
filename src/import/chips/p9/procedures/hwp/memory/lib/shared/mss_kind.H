/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/memory/lib/shared/mss_kind.H $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2020                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

///
/// @file mss_kind.H
/// @brief Implementation of mss specific types
///
// *HWP HWP Owner: Stephen Glancy <sglancy@us.ibm.com>
// *HWP HWP Backup: Andre Marin <aamarin@us.ibm.com>
// *HWP Team: Memory
// *HWP Level: 3
// *HWP Consumed by: HB:FSP

#ifndef _MSS_KIND_H_
#define _MSS_KIND_H_

#include <generic/memory/lib/utils/dimm/kind.H>

namespace mss
{

inline kind_t dimm_kind( const uint64_t i_type, const uint64_t i_gen )
{
    // This is the conditional needed to differentiate dimm type/generation
    switch (i_type)
    {
        case fapi2::ENUM_ATTR_EFF_DIMM_TYPE_RDIMM:
            if (i_gen == fapi2::ENUM_ATTR_EFF_DRAM_GEN_EMPTY)
            {
                return KIND_RDIMM_EMPTY;
            }

            if (i_gen == fapi2::ENUM_ATTR_EFF_DRAM_GEN_DDR4)
            {
                return KIND_RDIMM_DDR4;
            }

            return DEFAULT_KIND;
            break;

        case fapi2::ENUM_ATTR_EFF_DIMM_TYPE_LRDIMM:
            if (i_gen == fapi2::ENUM_ATTR_EFF_DRAM_GEN_EMPTY)
            {
                return KIND_LRDIMM_EMPTY;
            }

            if (i_gen == fapi2::ENUM_ATTR_EFF_DRAM_GEN_DDR4)
            {
                return KIND_LRDIMM_DDR4;
            }

            return DEFAULT_KIND;
            break;

        default:
            return DEFAULT_KIND;
            break;
    };
}

}
#endif
