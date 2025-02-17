/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/fapi2/plat_utils.H $                          */
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
/// @file plat_utils.H
///
/// @brief FAPI2 utility functions
///
/// Note that platform code must provide the implementation.
///

#ifndef PLAT_UTILS_H_
#define PLAT_UTILS_H_

#include <return_code.H>
#include <common_ringId.H>

namespace fapi2
{

// max ring size in centaur is 76490 bits - allocate a 10k byte
// buffer as the max size
#define MAX_CENTAUR_RING_SIZE 10 * 1024

///
/// @brief Add error log pointer as data to the ReturnCode and also add it
///        to the global error log list so we can delete it later
///
/// @param[in]  i_rc - ReturnCode reference
/// @param[in]  i_err - Error log pointer
///
void addErrlPtrToReturnCode(fapi2::ReturnCode& i_rc,
                             errlHndl_t i_err);

///
///  @brief This function answers the question, is scand tracing turned on?
///  The implementation of this function is specific to the platform.
///
///  @returns Boolean indication
///
bool platIsScanTraceEnabled();

///
/// @brief Converts a fapi2::ReturnCode to a HostBoot PLAT error log
///
/// @param[io] io_rc Reference to ReturnCode (set to success after function)
/// @param[in] i_sev Error log severity defaulted to unrecoverable
///
/// @return errlHndl_t (NULL if Return Code contained success)
///
errlHndl_t rcToErrl(ReturnCode & io_rc,
                    ERRORLOG::errlSeverity_t i_sev =
                        ERRORLOG::ERRL_SEV_UNRECOVERABLE);


///
/// @brief Enable/Disable special wakeup on processor chip core(s)
///
/// Special Wakeup Enable must be done when a HWP is doing an operation that
/// requires core(s) to be awake (e.g. modifying the Hcode image). For
/// each Special Wakeup Enable call, there must be a subsequent Special Wakeup
/// Disable call.
///
/// @param[in] i_target
///              TARGET_TYPE_PROC_CHIP: Enables/Disables Special Wakeup on all
///                cores (EX,EQ chiplets) of the specified chip target.
///              TARGET_TYPE_CORE: Enables/Disables Special Wakeup on the
///                specified core target (EX,EQ chiplets)
///              TARGET_TYPE_EX: Enables/Disables Special Wakeup on the
///                specified EX target.
///              TARGET_TYPE_EQ: Enables/Disables Special Wakeup on the
///                specified EQ target.
///
/// @param[in] i_enable true = enable. false = disable.
///
/// @return ReturnCode. FAPI2_RC_SUCCESS on success, else platform specified error.
///
///
ReturnCode platSpecialWakeup( const Target<TARGET_TYPE_ALL>& i_target,
                              bool i_enable );

///
/// @brief return uncompressed ring data from the hardware image for a
///        given ring ID.
///
/// @param[in] i_target -
///              TARGET_TYPE_MEMB_CHIP: uses centaur hw image data
/// @param[in] i_ringId      - which ring to get data for
/// @param[in] o_ringData    - ring data
/// @param[in] o_ringAddress - scan address
///
/// @return ReturnCode. FAPI2_RC_SUCCESS on success, else platform
///                     specified error.
///
/// NODE: Currenlty only supported for centaur membuf targets
///
template<fapi2::TargetType T>
ReturnCode get_ring(fapi2::Target<T> i_target,
        const RingId_t i_ringId,
        unsigned char *&o_ringData,
        uint64_t &o_ringLength,
        uint64_t &o_ringAddress);

///
/// @brief Returns child targets to Callout/Deconfigure/GARD
///
/// @param[i] i_parentTarget FAPI2 Parent Target
/// @param[i] i_childType    FAPI2 Child Type
/// @param[i] i_childPort    Child Port Number
///                            For DIMMs: MBA Port Number
///                            Else unused
/// @param[i] i_childNum     Child Number
///                            For DIMMs: DIMM Socket Number
///                            For Chips: Chip Position
///                            For Chiplets: Chiplet Position
/// @param[o] o_childTargets List of child targets matching input
//                             criteria.
///
void getChildTargetsForCDG(
             const fapi2::Target<fapi2::TARGET_TYPE_ALL>& i_parentTarget,
             const fapi2::TargetType i_childType,
             const uint8_t i_childPort,
             const uint8_t i_childNum,
             TARGETING::TargetHandleList & o_childTargets);

} // End namespace fapi2


#endif // PLAT_UTILS_H_
