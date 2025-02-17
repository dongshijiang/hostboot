/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/kernel/bltohbdatamgr.C $                                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2020                        */
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
#include <kernel/bltohbdatamgr.H>
#include <util/align.H>
#include <kernel/console.H>
#include <assert.h>
#include <arch/memorymap.H>
#include <bootloader/bootloaderif.H>
#include <kernel/vmmmgr.H>

// Global and only BlToHbDataManager instance
BlToHbDataManager g_BlToHbDataManager;

////////////////////////////////////////////////////////////////////////////////
//--------------------------------- Private ----------------------------------//
////////////////////////////////////////////////////////////////////////////////

// Set static variables to control use
Bootloader::BlToHbData BlToHbDataManager::iv_data;
bool BlToHbDataManager::iv_instantiated = false;
bool BlToHbDataManager::iv_initialized = false;
bool BlToHbDataManager::iv_dataValid = false;
size_t BlToHbDataManager::iv_preservedSize = 0;

void BlToHbDataManager::print() const
{
    if(iv_dataValid)
    {
        printkd("\nBlToHbData (all addr HRMOR relative):\n");
        if(iv_data.version >= Bootloader::BLTOHB_SAB)
        {
            printkd("-- secureSettings: SAB=%d, SecOvrd=%d, AllowAttrOvrd=%d\n",
                    iv_data.secureAccessBit, iv_data.securityOverride,
                    iv_data.allowAttrOverrides);
        }

        printkd("-- eyeCatch = 0x%lX (%s)\n", iv_data.eyeCatch,
                                    reinterpret_cast<char*>(&iv_data.eyeCatch));
        printkd("-- version = 0x%lX\n", iv_data.version);
        printkd("-- branchtableOffset = 0x%lX\n", iv_data.branchtableOffset);
        printkd("-- SecureRom Addr = 0x%lX Size = 0x%lX\n", getSecureRomAddr(),
               iv_data.secureRomSize);
        printkd("-- HW keys' Hash Addr = 0x%lX Size = 0x%lX\n",
                getHwKeysHashAddr(),
                iv_data.hwKeysHashSize);
        if(iv_data.version >= Bootloader::BLTOHB_SECURE_VERSION)
        {
            printkd("-- Minimum FW Secure Version  = 0x%02X\n",
                    iv_data.min_secure_version);
        }
        printkd("-- HBB header Addr = 0x%lX Size = 0x%lX\n", getHbbHeaderAddr(),
               iv_data.hbbHeaderSize);
        printkd("-- Reserved Size = 0x%lX\n", iv_preservedSize);
        if(iv_data.version >= Bootloader::BLTOHB_SIZE)
        {
            printkd("-- Size of structure = 0x%lX\n", iv_data.sizeOfStructure);
        }
        printkd("\n");
    }
}

////////////////////////////////////////////////////////////////////////////////
//---------------------------------- Public ----------------------------------//
////////////////////////////////////////////////////////////////////////////////

BlToHbDataManager::BlToHbDataManager()
{
    // Allow only one instantiation
    if (iv_instantiated)
    {
        printk("E> A BlToHbDataManager class instance already exists\n");
        kassert(!iv_instantiated);
    }
    iv_instantiated = true;
}

void BlToHbDataManager::initValid (const Bootloader::BlToHbData& i_data)
{
    // Allow only one initializer call
    if (iv_initialized)
    {
        printk("E> BlToHbDataManager class previously initialized\n");
        kassert(!iv_initialized);
    }

    // Simple assertion checks
    kassert(i_data.eyeCatch>0);
    kassert(i_data.version>0);
    kassert(i_data.branchtableOffset>0);
    kassert(i_data.secureRom!=nullptr);
    kassert(i_data.hwKeysHash!=nullptr);
    kassert(i_data.hbbHeader!=nullptr);
    kassert(i_data.secureRomSize>0);
    kassert(i_data.hwKeysHashSize>0);
    kassert(i_data.hbbHeaderSize>0);

    // Set internal static data
    iv_data.eyeCatch = i_data.eyeCatch;
    iv_data.version = i_data.version;
    iv_data.branchtableOffset = i_data.branchtableOffset;
    iv_data.secureRom = i_data.secureRom;
    iv_data.secureRomSize = i_data.secureRomSize;
    iv_data.hwKeysHash = i_data.hwKeysHash;
    iv_data.hwKeysHashSize = i_data.hwKeysHashSize;
    iv_data.hbbHeader = i_data.hbbHeader;
    iv_data.hbbHeaderSize = i_data.hbbHeaderSize;

printk("Version=%lX\n",i_data.version);
    // Ensure Bootloader to HB structure has the Secure Settings
    if(iv_data.version >= Bootloader::BLTOHB_SAB)
    {
        iv_data.secureAccessBit = i_data.secureAccessBit;
    }

    if(iv_data.version >= Bootloader::BLTOHB_SECURE_OVERRIDES)
    {
        iv_data.securityOverride   = i_data.securityOverride;
        iv_data.allowAttrOverrides = i_data.allowAttrOverrides;
    }
    else
    {
        iv_data.securityOverride   = 0;
        iv_data.allowAttrOverrides = 0;
    }

    // Ensure Bootloader to HB structure has the MMIO members
    // NOTE: BLTOHB_MMIOBARS version may not be sufficient to ensure that BARs
    //       were set correctly, instead use BLTOHB_SECURE_OVERRIDES version.
    if( iv_data.version >= Bootloader::BLTOHB_SECURE_OVERRIDES )
    {
        kassert(i_data.lpcBAR>0);
        kassert(i_data.xscomBAR>0);
        iv_data.lpcBAR = i_data.lpcBAR;
        iv_data.xscomBAR = i_data.xscomBAR;
    }
    else
    {
        //default to group0-proc0 values for down-level SBE
        iv_data.lpcBAR = MMIO_GROUP0_CHIP0_LPC_BASE_ADDR;
        iv_data.xscomBAR = MMIO_GROUP0_CHIP0_XSCOM_BASE_ADDR;

    }
    printk("lpc=%lX, xscom=%lX\n", i_data.lpcBAR, i_data.xscomBAR );

    printk("iv_lpc=%lX, iv_xscom=%lX, iv_data=%p\n",
            iv_data.lpcBAR, iv_data.xscomBAR, static_cast<void *>(&iv_data) );

    // Check if bootloader advertised the size of the structure it saw;
    // otherwise use the default padded size
    if(iv_data.version >= Bootloader::BLTOHB_SIZE)
    {
        iv_data.sizeOfStructure = i_data.sizeOfStructure;
    }
    else
    {
        iv_data.sizeOfStructure = Bootloader::INITIAL_BLTOHB_PADDED_SIZE;
    }

    if(iv_data.version >= Bootloader::BLTOHB_KEYADDR)
    {
        memcpy(&iv_data.keyAddrStashData,
               &i_data.keyAddrStashData,
               sizeof(Bootloader::keyAddrPair_t));
    }

    if(iv_data.version >= Bootloader::BLTOHB_BACKDOOR)
    {
        iv_data.secBackdoorBit = i_data.secBackdoorBit;
    }

    if(iv_data.version >= Bootloader::BLTOHB_SECURE_VERSION)
    {
        iv_data.min_secure_version = i_data.min_secure_version;
    }
    else
    {
        // Set to zero as default for older BLTOHB versions
        iv_data.min_secure_version = 0;
    }

    // Size of data that needs to be preserved and pinned.
    iv_preservedSize = ALIGN_PAGE(iv_data.secureRomSize +
                                 iv_data.hwKeysHashSize +
                                 iv_data.hbbHeaderSize );

    // Move preserved content to a location free from cache clearing
    relocatePreservedArea();

    iv_initialized = true;
    iv_dataValid = true;

    print();
}

void BlToHbDataManager::initInvalid ()
{
    printkd("BlToHbDataManager::initInvalid\n");
    // Allow only one initializer call
    if (iv_initialized)
    {
        printk("E> BlToHbDataManager class previously initialized\n");
        kassert(!iv_initialized);
    }

    //default to group0-proc0 values for down-level SBE
    iv_data.lpcBAR = MMIO_GROUP0_CHIP0_LPC_BASE_ADDR;
    iv_data.xscomBAR = MMIO_GROUP0_CHIP0_XSCOM_BASE_ADDR;

    iv_initialized = true;
    iv_dataValid = false;
    print();
}

void BlToHbDataManager::relocatePreservedArea()
{
    // Allow call this within the initializer
    if (iv_initialized)
    {
        printk("E> BlToHbDataManager relocatePreservedArea called outside initializer\n");
        kassert(!iv_initialized);
    }
    // Ensure the pointers were initialized
    kassert(iv_data.secureRom!=nullptr);
    kassert(iv_data.hwKeysHash!=nullptr);
    kassert(iv_data.hbbHeader!=nullptr);

    // Get destination location that will be preserved by the pagemgr
    auto l_pBltoHbDataStart = reinterpret_cast<uint8_t *>(
                                    VmmManager::BlToHbPreserveDataOffset());
    // Copy in SecureRom
    memcpy(l_pBltoHbDataStart,
           iv_data.secureRom,
           iv_data.secureRomSize);
    // Change pointer to new location and increment
    iv_data.secureRom = l_pBltoHbDataStart;
    l_pBltoHbDataStart += iv_data.secureRomSize;

    // Copy in HW keys' Hash
    memcpy(l_pBltoHbDataStart,
           iv_data.hwKeysHash,
           iv_data.hwKeysHashSize);
    // Change pointer to new location and increment
    iv_data.hwKeysHash = l_pBltoHbDataStart;
    l_pBltoHbDataStart += iv_data.hwKeysHashSize;

    // Copy in HBB header
    memcpy(l_pBltoHbDataStart,
           iv_data.hbbHeader,
           iv_data.hbbHeaderSize);
    // Change pointer to new location
    iv_data.hbbHeader = l_pBltoHbDataStart;
}

const uint64_t BlToHbDataManager::getBranchtableOffset() const
{
    return iv_data.branchtableOffset;
}

const void* BlToHbDataManager::getSecureRom() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access SecureRom\n");
        crit_assert(iv_dataValid);
    }
    return iv_data.secureRom;
}

const uint64_t BlToHbDataManager::getSecureRomAddr() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access SecureRomAddr\n");
        crit_assert(iv_dataValid);
    }
    return reinterpret_cast<uint64_t>(iv_data.secureRom);
}

const size_t BlToHbDataManager::getSecureRomSize() const
{
    return iv_data.secureRomSize;
}

const void* BlToHbDataManager::getHwKeysHash() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access HwKeysHash\n");
        crit_assert(iv_dataValid);
    }
    return iv_data.hwKeysHash;
}

const uint64_t BlToHbDataManager::getHwKeysHashAddr() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access HwKeysHashAddr\n");
        crit_assert(iv_dataValid);
    }
    return reinterpret_cast<uint64_t>(iv_data.hwKeysHash);
}

const size_t BlToHbDataManager::getHwKeysHashSize() const
{
    return iv_data.hwKeysHashSize;
}

const uint8_t BlToHbDataManager::getMinimumSecureVersion() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access Minimum Secure Version\n");
        crit_assert(iv_dataValid);
    }
    return iv_data.min_secure_version;
}


const void* BlToHbDataManager::getHbbHeader() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access HbbHeader\n");
        crit_assert(iv_dataValid);
    }
    return iv_data.hbbHeader;
}

const uint64_t BlToHbDataManager::getHbbHeaderAddr() const
{
    if(!iv_dataValid)
    {
        printk("E> BlToHbDataManager is invalid, cannot access HbbHeaderAddr\n");
        crit_assert(iv_dataValid);
    }
    return reinterpret_cast<uint64_t>(iv_data.hbbHeader);
}

const size_t BlToHbDataManager::getHbbHeaderSize() const
{
    return iv_data.hbbHeaderSize;
}

const bool BlToHbDataManager::getSecureAccessBit() const
{
    return iv_data.secureAccessBit;
}

const bool BlToHbDataManager::getSecurityOverride() const
{
    return iv_data.securityOverride;
}

const bool BlToHbDataManager::getAllowAttrOverrides() const
{
    return iv_data.allowAttrOverrides;
}

const size_t BlToHbDataManager::getPreservedSize() const
{
    return iv_preservedSize;
}

const bool BlToHbDataManager::isValid() const
{
    return iv_dataValid;
}

const uint64_t BlToHbDataManager::getLpcBAR() const
{
    return reinterpret_cast<uint64_t>(iv_data.lpcBAR);
}

const uint64_t BlToHbDataManager::getXscomBAR() const
{
    return reinterpret_cast<uint64_t>(iv_data.xscomBAR);
}

const Bootloader::keyAddrPair_t BlToHbDataManager::getKeyAddrPairs() const
{
    return iv_data.keyAddrStashData;
}

const size_t BlToHbDataManager::getBlToHbDataSize() const
{
    return iv_data.sizeOfStructure;
}

const bool BlToHbDataManager::getSecBackdoor() const
{
    return iv_data.secBackdoorBit;
}

