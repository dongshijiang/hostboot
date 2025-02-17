/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/io/p9a_io_omi_scominit.C $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
/// @file p9a_io_omi_scominit.C
/// @brief Invoke OMI initfile
///
//----------------------------------------------------------------------------
// *HWP HWP Owner       : Chris Steffen <cwsteffen@us.ibm.com>
// *HWP HWP Backup Owner: Gary Peterson <garyp@us.ibm.com>
// *HWP FW Owner        : Sumit Kumar <sumit_kumar@in.ibm.com>
// *HWP Team            : IO
// *HWP Level           : 3
// *HWP Consumed by     : FSP:HB
//----------------------------------------------------------------------------
//
// @verbatim
// High-level procedure flow:
//
//   Invoke OMI scominit file.
//
// Procedure Prereq:
//   - System clocks are running.
// @endverbatim
//----------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------
#include <p9_io_scom.H>
#include <p9_io_regs.H>
#include <p9a_omi_io_scom.H>
#include <p9a_omic_io_scom.H>
#include <p9a_io_omi_scominit.H>
#include <p9_obus_scom_addresses.H>
#include <p9a_mc_scom_addresses.H>
#include <p9_obus_fir_utils.H>

//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------

// HWP entry point, comments in header
fapi2::ReturnCode p9a_io_omi_scominit(const fapi2::Target<fapi2::TARGET_TYPE_OMIC>& i_target)
{
    // mark HWP entry
    FAPI_INF("p9a_io_omi_scominit: Entering...");
    const uint8_t GROUP_00 = 0;
    const uint8_t LANE_00  = 0;
    const uint8_t SET_RESET = 1;
    const uint8_t CLEAR_RESET = 0;
    fapi2::ReturnCode rc = fapi2::FAPI2_RC_SUCCESS;

    // get system target
    const fapi2::Target<fapi2::TARGET_TYPE_SYSTEM> l_system_target;

    // get a proc target
    fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> l_proc_target = i_target.getParent<fapi2::TARGET_TYPE_PROC_CHIP>();

    auto l_omi_lst = i_target.getChildren<fapi2::TARGET_TYPE_OMI>();

    // assert IO reset to power-up bus endpoint logic
    FAPI_TRY(io::rmw(OPT_IORESET_HARD_BUS0, i_target, GROUP_00, LANE_00, SET_RESET));

    // Bus Reset is relatively fast, only needing < a hundred cycles to allow the signal to propogate.
    FAPI_TRY(fapi2::delay(10, 1000));

    FAPI_TRY(io::rmw(OPT_IORESET_HARD_BUS0, i_target, GROUP_00, LANE_00, CLEAR_RESET));

    FAPI_INF("Invoke FAPI procedure core: input_target");
    FAPI_EXEC_HWP(rc, p9a_omic_io_scom, i_target, l_system_target, l_proc_target);

    for (auto l_omi_target : l_omi_lst)
    {
        FAPI_EXEC_HWP(rc, p9a_omi_io_scom, l_omi_target, l_system_target, l_proc_target);
    }

    // configure FIR if p9a_omi_io_scom passed
    if ( fapi2::current_err == fapi2::FAPI2_RC_SUCCESS )
    {
        FAPI_TRY(fapi2::putScom(i_target,
                                P9A_OMIC_FIR_ACTION0_REG,
                                OBUS_PHY_FIR_ACTION0),
                 "Error from putScom (P9A_OMIC_FIR_ACTION0_REG)");
        FAPI_TRY(fapi2::putScom(i_target,
                                P9A_OMIC_FIR_ACTION1_REG,
                                OBUS_PHY_FIR_ACTION1),
                 "Error from putScom (P9A_OMIC_FIR_ACTION1_REG)");
        FAPI_TRY(fapi2::putScom(i_target,
                                P9A_OMIC_FIR_MASK_REG,
                                OBUS_PHY_FIR_MASK),
                 "Error from putScom (P9A_OMIC_FIR_MASK_REG)");
    }

    // mark HWP exit
    FAPI_INF("p9a_io_omi_scominit: ...Exiting, rc = 0x%X",
             (uint32_t)fapi2::current_err);

fapi_try_exit:
    return fapi2::current_err;
}
