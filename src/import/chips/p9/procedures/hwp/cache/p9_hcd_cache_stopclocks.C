/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/import/chips/p9/procedures/hwp/cache/p9_hcd_cache_stopclocks.C $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
/// @file  p9_hcd_cache_stopclocks.C
/// @brief Quad Clock Stop
///
/// Procedure Summary:

// *HWP HWP Owner          : David Du       <daviddu@us.ibm.com>
// *HWP Backup HWP Owner   : Greg Still     <stillgs@us.ibm.com>
// *HWP FW Owner           : Amit Tendolkar <amit.tendolkar@in.ibm.com>
// *HWP Team               : PM
// *HWP Consumed by        : HB:PERV
// *HWP Level              : 3

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <p9_misc_scom_addresses.H>
#include <p9_quad_scom_addresses.H>
#include <p9_quad_scom_addresses_fld.H>
#include <p9_hcd_common.H>
#include <p9_common_clk_ctrl_state.H>
#include <p9_hcd_l2_stopclocks.H>
#include <p9_hcd_cache_stopclocks.H>
#include <p9_eq_clear_atomic_lock.H>
#ifdef __PPE__
    #include <p9_sbe_ppe_utils.H>
#else
    #include <p9_ppe_utils.H>
#endif
#include <p9_ppe_defs.H>

//------------------------------------------------------------------------------
// Constant Definitions
//------------------------------------------------------------------------------
enum P9_HCD_CACHE_STOPCLOCKS_CONSTANTS
{
    CACHE_CLK_STOP_POLLING_HW_NS_DELAY     = 10000,
    CACHE_CLK_STOP_POLLING_SIM_CYCLE_DELAY = 320000,
    PB_PURGE_CACHE_STOP_POLLING_DELAY_HW_MILLISEC = 1000000ULL, // 1msec
    PB_PURGE_CACHE_STOP_POLLING_DELAY_SIM_CYCLES = 10000ULL,
    PB_PURGE_CACHE_STOP_POLLING_TIMEOUT = 500,  // @ 1ms polling -> 500ms
};

//------------------------------------------------------------------------------
// Procedure: Quad Clock Stop
//------------------------------------------------------------------------------
// See doxygen in header file
fapi2::ReturnCode
p9_hcd_cache_stopclocks(
    const fapi2::Target<fapi2::TARGET_TYPE_EQ>& i_target,
    const p9hcd::P9_HCD_CLK_CTRL_CONSTANTS      i_select_regions,
    const p9hcd::P9_HCD_EX_CTRL_CONSTANTS       i_select_ex,
    const bool i_sync_stop_quad_clk)
{
    FAPI_INF(">>p9_hcd_cache_stopclocks: regions[%016llx] ex[%d]",
             i_select_regions, i_select_ex);
    fapi2::ReturnCode                              l_rc;
    fapi2::buffer<uint64_t>                        l_data64;
    fapi2::buffer<uint64_t>                        l_temp64;
    uint64_t                                       l_region_clock              = 0;
    uint64_t                                       l_region_fence              = 0;
    uint64_t                                       l_l3mask_pscom              = 0;
    uint32_t                                       l_loops1ms                  = 0;
    uint32_t                                       l_scom_addr                 = 0;
    uint8_t                                        l_attr_chip_unit_pos        = 0;
    uint8_t                                        l_attr_chip_core_pos = 0;
    uint8_t                                        l_attr_vdm_enabled           = 0;
    uint8_t                                        l_is_mpipl                  = 0;
    uint8_t                                        l_eq_pos;
    const fapi2::Target<fapi2::TARGET_TYPE_SYSTEM> l_sys;
    auto l_perv = i_target.getParent<fapi2::TARGET_TYPE_PERV>();
    auto l_chip = i_target.getParent<fapi2::TARGET_TYPE_PROC_CHIP>();
    auto l_core_functional_vector =
        i_target.getChildren<fapi2::TARGET_TYPE_CORE>
        (fapi2::TARGET_STATE_FUNCTIONAL);
    auto l_ex_vector =
        i_target.getChildren<fapi2::TARGET_TYPE_EX>
        (fapi2::TARGET_STATE_FUNCTIONAL);

    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_IS_MPIPL, l_sys, l_is_mpipl));
    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_CHIP_UNIT_POS, l_perv,
                           l_attr_chip_unit_pos));
    //  l_attr_chip_unit_pos = l_attr_chip_unit_pos - p9hcd::PERV_TO_QUAD_POS_OFFSET;
    l_attr_chip_unit_pos = l_attr_chip_unit_pos - 0x10;



    //Check if EQ is powered off; if so, return
    FAPI_TRY(fapi2::getScom(i_target, EQ_PPM_PFSNS, l_data64),
             "Error reading data from EQ_PPM_PFSNS");

    if(l_data64.getBit<EQ_PPM_PFSNS_VDD_PFETS_DISABLED_SENSE>())
    {
        return fapi2::current_err;
    }

    if(l_is_mpipl)
    {
        // PB_PURGE related SCOMs should be added to the beginning of
        // p9_hcd_cache_stopclocks, these scoms should only be done if the clock
        // region including PBIEQ clock domain is being stopped, which
        // incidentally should always be the case for MPIPL
        l_data64.flush<0>();
        l_data64.setBit<EQ_QPPM_QCCR_PB_PURGE_REQ>();
        // Set bit 30 in EQ_QPPM_QCCR_SCOM2(100F01BF) Reg, Pulse to the
        // Powerbus logic in the Cache clock domain to request them to purge
        // their async buffers in preparation to power off the Quad
        FAPI_TRY(fapi2::putScom(i_target, EQ_QPPM_QCCR_SCOM2, l_data64));

        l_data64.flush<0>();
        uint32_t l_timeout = PB_PURGE_CACHE_STOP_POLLING_TIMEOUT;

        do
        {
            // Acknowledgement from Powerbus that the buffers are empty
            // and can safely be fenced & clocked off.
            FAPI_TRY(fapi2::getScom(i_target, EQ_QPPM_QCCR_SCOM, l_data64));

            if(l_data64.getBit<EQ_QPPM_QCCR_PB_PURGE_DONE_LVL>())
            {
                break;
            }

            FAPI_TRY(fapi2::delay(PB_PURGE_CACHE_STOP_POLLING_DELAY_HW_MILLISEC,
                                  PB_PURGE_CACHE_STOP_POLLING_DELAY_SIM_CYCLES),
                     "Error from delay"); //1msec delay
        }
        while(--l_timeout != 0);

        // Error if Timeout happens
        FAPI_ASSERT((l_timeout != 0),
                    fapi2::QPPM_QCCR_PB_PURGE_DONE_LVL_TIMEOUT()
                    .set_TARGET(i_target)
                    .set_EQPPMQCCR(l_data64),
                    "QPPM_QCCR_PB_PURGE_DONE_LVL Reg bit _QPPM_QCCR_PB_PURGE_DONE_LVL not set.");

        // Clear purge request
        l_data64.flush<0>();
        l_data64.setBit<EQ_QPPM_QCCR_PB_PURGE_REQ>();
        FAPI_TRY(fapi2::putScom(i_target, EQ_QPPM_QCCR_SCOM1, l_data64));
    }

    FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_VDM_ENABLED, l_chip,
                           l_attr_vdm_enabled));

    if (i_select_regions & p9hcd::CLK_REGION_EX0_L3)
    {
        l_l3mask_pscom |= (BIT64(4) | BIT64(6) | BIT64(8));
    }

    if (i_select_regions & p9hcd::CLK_REGION_EX1_L3)
    {
        l_l3mask_pscom |= (BIT64(5) | BIT64(7) | BIT64(9));
    }

    // -----------------------------
    // Prepare to stop cache clocks
    // -----------------------------
    FAPI_DBG("Check PM_RESET_STATE_INDICATOR via GPMMR[15]");
    FAPI_TRY(getScom(i_target, EQ_PPM_GPMMR_SCOM, l_data64));

    if (!l_data64.getBit<EQ_PPM_GPMMR_RESET_STATE_INDICATOR>())
    {
        FAPI_DBG("Gracefully turn off power management, if fail, continue anyways");

#ifdef DD2
        FAPI_DBG("Halting the PGPE ...");
        l_rc = ppe_halt(l_chip, PGPE_BASE_ADDRESS);
        FAPI_ASSERT_NOEXIT(!l_rc,
                           fapi2::CACHE_STOPCLKS_PGPE_HALT_TIMEOUT()
                           .set_CHIP(l_chip),
                           "PSTATE GPE Halt timeout");

        FAPI_DBG("Halting the SGPE ...");
        l_rc = ppe_halt(l_chip, SGPE_BASE_ADDRESS);
        FAPI_ASSERT_NOEXIT(!l_rc,
                           fapi2::CACHE_STOPCLKS_SGPE_HALT_TIMEOUT()
                           .set_CHIP(l_chip),
                           "STOP GPE Halt timeout");

        FAPI_DBG("Clear the atomic lock on EQ %d", l_attr_chip_unit_pos);
        l_rc = p9_clear_atomic_lock(i_target);
        FAPI_DBG("AFter the atomic lock %u", (uint32_t)l_rc);
        FAPI_ASSERT_NOEXIT(!l_rc,
                           fapi2::CACHE_STOPCLKS_ATOMIC_LOCK_FAIL()
                           .set_EQ(i_target),
                           "EQ Atomic Halt timeout");

        for ( auto& ex : l_ex_vector )
        {
            fapi2::ATTR_CHIP_UNIT_POS_Type  l_cme_id = 0;
            FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_CHIP_UNIT_POS, ex, l_cme_id));

            FAPI_DBG("Halting CME %d", l_cme_id );
            uint64_t l_cme_base_address = getCmeBaseAddress (l_cme_id);
            l_rc = ppe_halt(l_chip, l_cme_base_address);
            FAPI_ASSERT_NOEXIT(!l_rc,
                               fapi2::CACHE_STOPCLKS_CME_HALT_TIMEOUT()
                               .set_EX(ex),
                               "CME Halt timeout");
        }

#endif
    }

    FAPI_DBG("Check cache clock controller status");
    l_rc = p9_common_clk_ctrl_state<fapi2::TARGET_TYPE_EQ>(i_target);

    if (l_rc)
    {
        FAPI_INF("Clock controller of this cache chiplet is inaccessible, return");
        goto qssr_update;
    }

    FAPI_DBG("Check PERV clock status for access to CME via CLOCK_STAT[4]");
    FAPI_TRY(getScom(i_target, EQ_CLOCK_STAT_SL, l_data64));

    FAPI_DBG("Check PERV fence status for access to CME via CPLT_CTRL1[4]");
    FAPI_TRY(getScom(i_target, EQ_CPLT_CTRL1, l_temp64));

    if (l_data64.getBit<EQ_CLOCK_STAT_SL_STATUS_PERV>() == 0 &&
        l_temp64.getBit<EQ_CPLT_CTRL1_TC_PERV_REGION_FENCE>() == 0)
    {
        FAPI_DBG("Assert L3 pscom masks via RING_FENCE_MASK_LATCH_REG[4-9]");
        FAPI_TRY(putScom(i_target, EQ_RING_FENCE_MASK_LATCH_REG, l_l3mask_pscom));
    }

    FAPI_DBG("Assert chiplet fence via NET_CTRL0[18]");
    FAPI_TRY(putScom(i_target, EQ_NET_CTRL0_WOR, MASK_SET(EQ_NET_CTRL0_FENCE_EN)));

    // -------------------------------
    // Stop L2 clocks
    // -------------------------------

    if (i_select_ex && !i_sync_stop_quad_clk)
        FAPI_EXEC_HWP(fapi2::current_err,
                      p9_hcd_l2_stopclocks,
                      i_target, i_select_ex);

    // -------------------------------
    // Stop cache clocks
    // -------------------------------

    FAPI_DBG("Clear all SCAN_REGION_TYPE bits");
    FAPI_TRY(putScom(i_target, EQ_SCAN_REGION_TYPE, MASK_ZERO));

    l_region_clock = i_select_regions;

    if(i_sync_stop_quad_clk)
    {
        if (i_select_ex & p9hcd::EVEN_EX)
        {
            l_region_clock |= p9hcd::CLK_REGION_EX0_L2;
        }

        if (i_select_ex & p9hcd::ODD_EX)
        {
            l_region_clock |= p9hcd::CLK_REGION_EX1_L2;
        }

        FAPI_DBG("Stop cache clocks via CLK_REGION in master mode to perform stop quad clocks synchronously");
        l_data64 = (p9hcd::CLK_STOP_CMD_MASTER  |
                    l_region_clock  |
                    p9hcd::CLK_THOLD_ALL);
    }
    else
    {
        FAPI_DBG("Stop cache clocks via CLK_REGION");
        l_data64 = (p9hcd::CLK_STOP_CMD  |
                    l_region_clock       |
                    p9hcd::CLK_THOLD_ALL);
    }

    FAPI_TRY(putScom(i_target, EQ_CLK_REGION, l_data64));

    FAPI_DBG("Poll for cache clocks stopped via CPLT_STAT0[8]");
    l_loops1ms = 1E6 / CACHE_CLK_STOP_POLLING_HW_NS_DELAY;

    do
    {
        fapi2::delay(CACHE_CLK_STOP_POLLING_HW_NS_DELAY,
                     CACHE_CLK_STOP_POLLING_SIM_CYCLE_DELAY);

        FAPI_TRY(getScom(i_target, EQ_CPLT_STAT0, l_data64));
    }
    while((!l_data64.getBit<EQ_CPLT_STAT0_CC_CTRL_OPCG_DONE_DC>()) && ((--l_loops1ms) != 0));

    FAPI_ASSERT((l_loops1ms != 0),
                fapi2::PMPROC_CACHECLKSTOP_TIMEOUT()
                .set_TARGET(i_target)
                .set_EQCPLTSTAT(l_data64),
                "Cache Clock Stop Timeout");

    FAPI_DBG("Check cache clocks stopped");
    FAPI_TRY(getScom(i_target, EQ_CLOCK_STAT_SL, l_data64));

    FAPI_ASSERT((((~l_data64) & l_region_clock) == 0),
                fapi2::PMPROC_CACHECLKSTOP_FAILED()
                .set_TARGET(i_target)
                .set_EQCLKSTAT(l_data64),
                "Cache Clock Stop Failed");
    FAPI_DBG("Cache clocks stopped now");

    // -------------------------------
    // Fence up
    // -------------------------------

    FAPI_DBG("Assert vital fence via CPLT_CTRL1[3]");
    FAPI_TRY(putScom(i_target, EQ_CPLT_CTRL1_OR, MASK_SET(EQ_CPLT_CTRL1_TC_VITL_REGION_FENCE)));

    l_region_fence = l_region_clock;

    if (!l_is_mpipl)
    {
        if (l_region_fence & p9hcd::CLK_REGION_EX0_L3)
        {
            l_region_fence |= p9hcd::CLK_REGION_EX0_REFR;
        }

        if (l_region_fence & p9hcd::CLK_REGION_EX1_L3)
        {
            l_region_fence |= p9hcd::CLK_REGION_EX1_REFR;
        }
    }

    FAPI_DBG("Assert regional fences via CPLT_CTRL1[4-14]");
    FAPI_TRY(putScom(i_target, EQ_CPLT_CTRL1_OR, l_region_fence));

    // Gate the PCBMux request so scanning doesn't cause random requests
    for(auto& it : l_core_functional_vector)
    {
        FAPI_TRY(FAPI_ATTR_GET(fapi2::ATTR_CHIP_UNIT_POS,
                               it.getParent<fapi2::TARGET_TYPE_PERV>(),
                               l_attr_chip_core_pos));
        FAPI_DBG("Assert core[%d] PCB Mux Disable via C_SLAVE_CONFIG[7]",
                 (l_attr_chip_core_pos - p9hcd::PERV_TO_CORE_POS_OFFSET));
        l_scom_addr = (C_SLAVE_CONFIG_REG + (0x1000000 *
                                             (l_attr_chip_core_pos - p9hcd::PERV_TO_CORE_POS_OFFSET)));
        FAPI_TRY(getScom(l_chip, l_scom_addr, l_data64));
        FAPI_TRY(putScom(l_chip, l_scom_addr, DATA_SET(7)));
    }


    // -------------------------------
    // Shutdown edram
    // -------------------------------
    // QCCR[0/4] EDRAM_ENABLE_DC
    // QCCR[1/5] EDRAM_VWL_ENABLE_DC
    // QCCR[2/6] L3_EX0/1_EDRAM_VROW_VBLH_ENABLE_DC
    // QCCR[3/7] EDRAM_VPP_ENABLE_DC

    if (l_region_clock & p9hcd::CLK_REGION_EX0_REFR)
    {
        FAPI_DBG("Sequence EX0 EDRAM disables via QPPM_QCCR[0-3]");
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(3)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(2)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(1)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(0)));
    }

    if (l_region_clock & p9hcd::CLK_REGION_EX1_REFR)
    {
        FAPI_DBG("Sequence EX1 EDRAM disables via QPPM_QCCR[4-7]");
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(7)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(6)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(5)));
        FAPI_TRY(putScom(i_target, EQ_QPPM_QCCR_WCLEAR, MASK_SET(4)));
    }

qssr_update:
    // -------------------------------
    // Update QSSR and STOP history
    // -------------------------------
    l_eq_pos = l_attr_chip_unit_pos + 14;
    l_data64.flush<0>();
    l_data64.setBit(l_eq_pos);
    FAPI_DBG("Set cache as stopped in QSSR");
    FAPI_TRY(putScom(l_chip, PU_OCB_OCI_QSSR_OR, l_data64));

    FAPI_DBG("Set cache as stopped in STOP history register");
    FAPI_TRY(putScom(i_target, EQ_PPM_SSHSRC, BIT64(0)));

fapi_try_exit:

    FAPI_INF("<<p9_hcd_cache_stopclocks");
    return fapi2::current_err;
}
