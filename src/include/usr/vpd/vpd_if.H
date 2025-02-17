/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/vpd/vpd_if.H $                                */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2013,2019                        */
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
#if !defined(__VPD_IF_H)
#define __VPD_IF_H

#include <pnor/pnorif.H>
#include <map>

namespace VPD
{
    /**
     * @brief typdef for vpdRecord values
     */
    typedef uint32_t vpdRecord;
    /**
     * @brief typdef for vpdKeyword values
     */
    typedef uint32_t vpdKeyword;
    /**
     * @brief typdef for vpdCmdTarget values
     */
    enum vpdCmdTarget
    {
        AUTOSELECT         = 0x0000,

        // Use our cache or read the eeprom
        PNOR               = 0x0001,
        SEEPROM            = 0x0002,
        LOCATION_MASK      = 0x000F,

        // Use the real vpd or a firmware override
        USEVPD             = 0x0010,
        USEOVERRIDE        = 0x0020,
        OVERRIDE_MASK      = 0x00F0,

        INVALID_LOCATION   = 0xFFFF,
    };

    /**
     * @brief Load the runtime VPD image into memory
     * @param[in] The virtual address of the VPD image
     * @return error handle if there was an error
     */
    errlHndl_t vpd_load_rt_image(uint64_t & i_vpd_addr);

    /**
     * @brief This function checks to see if the given mvpd target
     *      is present
     * @param[in] i_target - Target device to search for mvpd
     * @return bool - true if mvpd is present, false if it is not.
     */
    bool mvpdPresent ( TARGETING::Target * i_target );

    /**
     * @brief This function checks to see if the given cvpd target
     *      is present
     * @param[in] i_target - Target device to search for cvpd
     * @return bool - true if cvpd is present, false if it is not.
     */
    bool cvpdPresent ( TARGETING::Target * i_target );


    /**
     * @brief This function sets the part and serial number attributes for
     *        a given target
     * @param[in] i_target - the target to set the attributes for
     *
     */
    void setPartAndSerialNumberAttributes( TARGETING::Target * i_target );

    /**
     * @brief This function checks to see if the given pvpd target
     *      is present
     * @param[in] i_target - Target device to search for pvpd
     * @return bool - true if pvpd is present, false if it is not.
     */
    bool pvpdPresent ( TARGETING::Target * i_target );

    /**
     * @brief This function checks if the PNOR cache for this target is in
     *      sync with the SEEPROM, if not it loads the PNOR cache from SEEPROM.
     * @param[in] i_target - Target device
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t ensureCacheIsInSync ( TARGETING::Target * i_target );

    /**
     * @brief This function checks if the eeprom cache for the target is in
     *        sync with hardware and returns the result in o_isInSync.
     *
     * @param[in]   i_target      Target device
     * @param[in]   i_eepromType  Eeprom content type of target device
     * @param[out]  o_isInSync    true if part and serial numbers in cache match
     *                            hardware. Otherwise, false.
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t ensureEepromCacheIsInSync(TARGETING::Target * i_target,
                            TARGETING::EEPROM_CONTENT_TYPE   i_eepromType,
                                         bool              & o_isInSync);

    /**
     * @brief This function invalidates the VPD data in the PNOR cache.
     * @param[in] i_target - Target device
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t invalidatePnorCache ( TARGETING::Target * i_target );

    /**
     * @brief This function validates targets sharing PNOR::CENTAUR_VPD cache.
     *        Invalidate sections where all of the targets sharing a VPD_REC_NUM
     *        are invalid. Keep the section if any target is valid.
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t validateSharedPnorCache( );

    /**
     * @brief This function invalidates all of the VPD data in the PNOR cache.
     * @param[in] i_setHwOnly - Also set vars to not access VPD cache
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t invalidateAllPnorCaches ( bool i_setHwOnly );

    /**
     * @brief This function checks for golden-side boot and updates
     *        the VPD SWITCHES attribute if true
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t goldenSwitchUpdate ( void );

    /**
     * @brief This function checks for manufacturing mode or
     *        golden-side boot and invalidates VPD PNOR cache if true
     * @return errlHndl_t - NULL if successful, otherwise a pointer to the
     *      error log.
     */
    errlHndl_t goldenCacheInvalidate ( void );

    /**
     * @brief Structure used to specify VPD overrides for runtime
     *        reserved memory usage
     */
    struct OverrideSpecifier_t
    {
        PNOR::SectionId pnorId;
        size_t size; // size in bytes
    };

    /**
     * @brief Map of override labels to sections
     *   index : uint64_t label = right-justified 4-digit ASCII string
     *   result : OverrideSpecifier_t
     */
    typedef std::map<uint64_t,OverrideSpecifier_t> OverrideRsvMemMap_t;

    /**
     * @brief Get a list of all overridden sections
     * @param[out]  List of known override sections
     */
    void getListOfOverrideSections( OverrideRsvMemMap_t& o_overrides );

}; //end vpd namespace

#endif
