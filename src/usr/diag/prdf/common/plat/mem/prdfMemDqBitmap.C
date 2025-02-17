/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/plat/mem/prdfMemDqBitmap.C $         */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2013,2020                        */
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

/** @file  prdfMemDqBitmap.C */

#include <prdfMemDqBitmap.H>

#include <UtilHash.H>
#include <iipServiceDataCollector.h>
#include <prdfParserUtils.H>

namespace PRDF
{

using namespace PlatServices;
using namespace PARSERUTILS;
using namespace TARGETING;
using namespace fapi2; // for spare dram config

bool MemDqBitmap::badDqs() const
{
    bool o_badDqs = false;

    size_t maxPorts = getNumPorts();

    for ( uint32_t i = 0; i < maxPorts; i++ )
    {
        for ( uint32_t j = 0; j < DQ_BITMAP::BITMAP_SIZE; j++ )
        {
            if ( 0 != iv_data.at(i).bitmap[j] )
            {
                o_badDqs = true;
                break;
            }
        }
        if ( o_badDqs ) break;
    }

    return o_badDqs;
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::badDqs( bool & o_badDqs, uint8_t i_portSlct ) const
{
    #define PRDF_FUNC "[MemDqBitmap::badDqs] "

    uint32_t o_rc = SUCCESS;

    o_badDqs = false;

    do
    {
        size_t maxPorts = getNumPorts();

        if ( maxPorts <= i_portSlct )
        {
            PRDF_ERR(PRDF_FUNC "Invalid parameter: i_portSlct=%d", i_portSlct);
            o_rc = FAIL; break;
        }

        for ( uint32_t j = 0; j < DQ_BITMAP::BITMAP_SIZE; j++ )
        {
            if ( 0 != iv_data.at(i_portSlct).bitmap[j] )
            {
                o_badDqs = true;
                break;
            }
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::setDq( uint8_t i_dq, uint8_t i_portSlct )
{
    #define PRDF_FUNC "[MemDqBitmap::setDq] "

    uint32_t o_rc = SUCCESS;

    do
    {
        uint8_t maxDqs = ( TYPE_OCMB_CHIP == getTargetType(iv_trgt) )
                         ? OCMB_DQS_PER_DIMM : DQS_PER_DIMM ;
        if ( maxDqs <= i_dq )
        {
            PRDF_ERR( PRDF_FUNC "Invalid parameter: i_dq=%d", i_dq );
            o_rc = FAIL; break;
        }

        size_t maxPorts = getNumPorts();

        if ( maxPorts <= i_portSlct )
        {
            PRDF_ERR(PRDF_FUNC "Invalid parameter: i_portSlct=%d", i_portSlct);
            o_rc = FAIL; break;
        }

        uint8_t byteIdx = i_dq / DQS_PER_BYTE;
        uint8_t bitIdx  = i_dq % DQS_PER_BYTE;

        uint32_t shift = (DQS_PER_BYTE-1) - bitIdx; // 0-7
        iv_data[i_portSlct].bitmap[byteIdx] |= 0x01 << shift;

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::setSymbol( const MemSymbol & i_symbol, uint8_t i_pins )
{
    #define PRDF_FUNC "[MemDqBitmap::setSymbol] "

    uint32_t o_rc = SUCCESS;

    do
    {
        uint8_t portSlct, byteIdx, bitIdx;
        o_rc = getPortByteBitIdx( i_symbol, portSlct, byteIdx, bitIdx );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getPortByteBitIdx() failed" );
            break;
        }

        uint32_t shift = (DQS_PER_BYTE-1) - bitIdx;

        // The number of dqs per symbol is equivalent to the ports we have here
        size_t dqsPerSym = getNumPorts();

        // Calculate pin mask -> (2^dqsPerSym)-1
        uint8_t pinMask = 2;
        for ( uint8_t i = 1; i < dqsPerSym; i++ ) {pinMask *= 2;}
        pinMask -= 1;

        i_pins &= pinMask; // Limit to the number of dqs per symbols
        shift = (shift / dqsPerSym) * dqsPerSym;

        iv_data[portSlct].bitmap[byteIdx] |= i_pins << shift;

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::setDram( const MemSymbol & i_symbol, uint8_t i_pins )
{
    #define PRDF_FUNC "[MemDqBitmap::setDram] "

    uint32_t o_rc = SUCCESS;

    do
    {
        uint8_t portSlct, byteIdx, bitIdx;
        o_rc = getPortByteBitIdx( i_symbol, portSlct, byteIdx, bitIdx );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getPortByteBitIdx() failed" );
            break;
        }

        if ( iv_x4Dram )
        {
            i_pins &= 0xf; // limit to 4 bits
            uint32_t shift = (DQS_PER_BYTE-1) - bitIdx;
            shift = (shift / DQS_PER_NIBBLE) * DQS_PER_NIBBLE; // 0,4
            iv_data[portSlct].bitmap[byteIdx] |= i_pins << shift;
        }
        else
        {
            i_pins &= 0xff; // limit to 8 bits
            iv_data[portSlct].bitmap[byteIdx] |= i_pins;
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::clearDram( const MemSymbol & i_symbol, uint8_t i_pins )
{
    #define PRDF_FUNC "[MemDqBitmap::clearDram] "

    uint32_t o_rc = SUCCESS;

    do
    {
        uint8_t portSlct, byteIdx, bitIdx;
        o_rc = getPortByteBitIdx( i_symbol, portSlct, byteIdx, bitIdx );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getPortByteBitIdx() failed" );
            break;
        }

        if ( iv_x4Dram )
        {
            i_pins &= 0xf; // limit to 4 bits
            uint32_t shift = (DQS_PER_BYTE-1) - bitIdx;
            shift = (shift / DQS_PER_NIBBLE) * DQS_PER_NIBBLE; // 0,4
            iv_data[portSlct].bitmap[byteIdx] &= ~(i_pins << shift);
        }
        else
        {
            i_pins &= 0xff; // limit to 8 bits
            iv_data[portSlct].bitmap[byteIdx] &= ~(i_pins);
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

void MemDqBitmap::clearBitmap()
{
    size_t maxPorts = getNumPorts();

    for ( uint32_t i = 0; i < maxPorts; i++ )
    {
        memset( iv_data[i].bitmap, 0x00, sizeof(iv_data[i].bitmap) );
    }
}

//------------------------------------------------------------------------------

void MemDqBitmap::getCaptureData( CaptureData & o_cd ) const
{
    uint8_t rank   = iv_rank.getMaster();
    size_t sz_rank = sizeof(rank);

    size_t sz_capData = sz_rank + sizeof(iv_data);

    // Adjust the size for endianness.
    const size_t sz_word = sizeof(CPU_WORD);
    sz_capData = ((sz_capData + sz_word-1) / sz_word) * sz_word;

    uint8_t capData[sz_capData];
    memset( capData, 0x00, sz_capData );

    capData[0] = rank;

    uint8_t numPorts = getNumPorts();
    uint8_t idx = 1;
    for ( uint8_t ps = 0; ps < numPorts; ps++ )
    {
        memcpy( &capData[idx], getData(ps), sizeof(capData[idx]) );
        idx += DQ_BITMAP::BITMAP_SIZE;
    }

    // Fix endianness issues with non PPC machines.
    for ( uint32_t i = 0; i < (sz_capData/sz_word); i++ )
        ((CPU_WORD*)capData)[i] = htonl(((CPU_WORD*)capData)[i]);

    BitString bs ( sz_capData*8, (CPU_WORD *) &capData );
    o_cd.Add( iv_trgt, Util::hashString("BAD_DQ_BITMAP"), bs );
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::getPortByteBitIdx( const MemSymbol & i_symbol,
                                        uint8_t & o_portSlct,
                                        uint8_t & o_byteIdx,
                                        uint8_t & o_bitIdx ) const
{
    #define PRDF_FUNC "[MemDqBitmap::getPortByteBitIdx] "

    uint32_t o_rc = SUCCESS;

    do
    {
        if ( !i_symbol.isValid() )
        {
            PRDF_ERR( PRDF_FUNC "i_symbol is invalid" );
            o_rc = FAIL; break;
        }

        o_portSlct = i_symbol.getPortSlct();
        o_byteIdx  = i_symbol.getDq() / DQS_PER_BYTE;
        o_bitIdx   = i_symbol.getDq() % DQS_PER_BYTE;

        if ( i_symbol.isDramSpared() )
        {
            o_byteIdx = iv_spareByte;
        }
        else if ( i_symbol.isEccSpared() )
        {
            o_portSlct = ECC_SPARE_PORT;
            o_byteIdx  = ECC_SPARE_BYTE;

            // x4 ECC spare is the second nibble of the byte.
            o_bitIdx = (o_bitIdx % DQS_PER_NIBBLE) + DQS_PER_NIBBLE;
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::isChipMark( const MemSymbol & i_symbol, bool & o_cm )
{
    #define PRDF_FUNC "[MemDqBitmap::isChipMark] "

    uint32_t o_rc = SUCCESS;
    o_cm = false;

    do
    {
        // If 2 or more symbols are set in a nibble, the chip mark is present.

        uint8_t portSlct, byteIdx, bitIdx;
        o_rc = getPortByteBitIdx( i_symbol, portSlct, byteIdx, bitIdx );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getPortByteBitIdx() failed" );
            break;
        }

        uint8_t cmData = iv_data[portSlct].bitmap[byteIdx];

        // x4 Drams
        if ( iv_x4Dram )
        {
            // Find which nibble to check.
            uint8_t nibble;
            if ( bitIdx < 4 )
                nibble = ( (cmData>>4) & 0xf );
            else
                nibble = cmData & 0xf;

            // This nibble must have 2 or more symbols set.
            o_cm = ( (0x0 != nibble) &&
                     (0x8 != nibble) &&
                     (0x4 != nibble) &&
                     (0x2 != nibble) &&
                     (0x1 != nibble) );
        }
        // x8 Drams
        else
        {
            uint32_t count = 0;
            while ( 0 != cmData )
            {
                if ( 0 != (cmData & 0x3) )
                    count++;

                cmData >>= 2;
            }
            o_cm = ( count >= 2 );
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

std::vector<MemSymbol> MemDqBitmap::getSymbolList( uint8_t i_portSlct )
{
    #define PRDF_FUNC "[MemDqBitmap::getSymbolList] "

    std::vector<MemSymbol> o_symbolList;

    size_t maxPorts = getNumPorts();

    // The number of dqs per symbol is equivalent to the ports we have here
    size_t dqsPerSymbol = getNumPorts();
    uint8_t symbolsPerByte = SYMBOLS_PER_RANK/(BYTES_PER_DIMM*dqsPerSymbol);

    // Calculate bit mask -> (2^dqsPerSym)-1
    uint8_t bitMask = 2;
    for ( uint8_t i = 1; i < dqsPerSymbol; i++ ) {bitMask *= 2;}
    bitMask -= 1;

    // loop through all ports
    for ( uint8_t port = 0; port < maxPorts; port++ )
    {
        // loop through each byte in the bitmap
        for ( uint8_t byte = 0; byte < DQ_BITMAP::BITMAP_SIZE; byte++ )
        {
            // loop through each symbol index
            for ( uint8_t symIdx = 0; symIdx < symbolsPerByte; symIdx++ )
            {
                uint8_t shift = ((symbolsPerByte - 1) - symIdx) * dqsPerSymbol;

                // if the bit/bit pair is active
                if ( ((iv_data[port].bitmap[byte] >> shift) & bitMask) != 0 )
                {
                    // get the dq
                    uint8_t dq = (byte * DQS_PER_BYTE)+(symIdx * dqsPerSymbol);

                    // convert the dq to symbol
                    uint8_t symbol =  SYMBOLS_PER_RANK;
                    TYPE trgtType = getTargetType( iv_trgt );
                    switch( trgtType )
                    {
                        case TYPE_MCA:
                            symbol = dq2Symbol<TYPE_MCA>( dq, i_portSlct );
                            break;
                        case TYPE_MBA:
                            symbol = dq2Symbol<TYPE_MBA>( dq, i_portSlct );
                            break;
                        case TYPE_MEM_PORT:
                            symbol = dq2Symbol<TYPE_MEM_PORT>( dq, i_portSlct );
                            break;
                        case TYPE_OCMB_CHIP:
                            symbol = dq2Symbol<TYPE_OCMB_CHIP>(dq, i_portSlct);
                            break;
                        default:
                            PRDF_ERR( "Invalid trgt type" );
                            PRDF_ASSERT( false );
                            break;
                    }

                    // add symbol to output
                    o_symbolList.push_back( MemSymbol::fromSymbol(iv_trgt,
                                            iv_rank, symbol) );
                }
            }
        }
    }

    return o_symbolList;

    #undef PRDF_FUNC

}

//------------------------------------------------------------------------------

uint32_t __getSpareInfo( TargetHandle_t i_trgt, MemRank i_rank,
    uint8_t i_portSlct, uint8_t & o_spareConfig, uint8_t & o_noSpare,
    uint8_t & o_lowNibble, uint8_t & o_highNibble, bool & o_spareSupported )
{
    #define PRDF_FUNC "[__getSpareInfo] "

    uint32_t o_rc = SUCCESS;
    o_spareSupported = true;

    do
    {
        TYPE trgtType = getTargetType( i_trgt );

        // Spares not supported on MCA
        if ( TYPE_MCA == trgtType )
        {
            o_spareSupported = false;
        }
        // Centaur/MBA case
        else if ( TYPE_MBA == trgtType )
        {
            o_noSpare     = CEN_VPD_DIMM_SPARE_NO_SPARE;
            o_lowNibble   = CEN_VPD_DIMM_SPARE_LOW_NIBBLE;
            o_highNibble  = CEN_VPD_DIMM_SPARE_HIGH_NIBBLE;
            o_spareConfig = CEN_VPD_DIMM_SPARE_NO_SPARE;
            o_rc = getDimmSpareConfig<TYPE_MBA>( i_trgt, i_rank, i_portSlct,
                                                 o_spareConfig );
        }
        // Generic/MEM_PORT case
        else
        {
            o_noSpare     = MEM_EFF_DIMM_SPARE_NO_SPARE;
            o_lowNibble   = MEM_EFF_DIMM_SPARE_LOW_NIBBLE;
            o_highNibble  = MEM_EFF_DIMM_SPARE_HIGH_NIBBLE;
            o_spareConfig = MEM_EFF_DIMM_SPARE_NO_SPARE;
            TargetHandle_t memPort = getConnectedChild( i_trgt, TYPE_MEM_PORT,
                                                        i_portSlct );
            o_rc = getDimmSpareConfig<TYPE_MEM_PORT>( memPort, i_rank,
                i_portSlct, o_spareConfig );
        }

        if( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getDimmSpareConfig() failed" );
            break;
        }
    }while(0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::isSpareAvailable( uint8_t i_portSlct, bool & o_dramSpare,
                                        bool & o_eccSpare )
{
    #define PRDF_FUNC "[MemDqBitmap::isSpareAvailable] "

    uint32_t o_rc = SUCCESS;
    o_dramSpare = false;
    o_eccSpare  = false;

    do
    {
        // Check to make sure the portSlct is valid
        size_t maxPorts = getNumPorts();
        if ( maxPorts <= i_portSlct )
        {
            PRDF_ERR( PRDF_FUNC "Invalid parameter: i_portSlct=%d", i_portSlct);
            o_rc = FAIL; break;
        }

        uint8_t spareConfig, noSpare, lowNibble, highNibble;
        bool spareSupported = true;
        o_rc = __getSpareInfo( iv_trgt, iv_rank, i_portSlct, spareConfig,
                               noSpare, lowNibble, highNibble, spareSupported );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "__getSpareInfo failed" );
            break;
        }

        // Spare is not available.
        if ( !spareSupported )
        {
            o_dramSpare = false;
            o_eccSpare  = false;
            break;
        }

        uint8_t spareDqBits = iv_data.at(i_portSlct).bitmap[iv_spareByte];

        if ( iv_x4Dram )
        {
            // Spare is on the lower nibble
            if ( lowNibble == spareConfig )
            {
                o_dramSpare = ( 0 == ( spareDqBits & 0xf0 ) );
            }
            // Spare is on the higher nibble
            else if ( highNibble == spareConfig )
            {
                o_dramSpare = ( 0 == ( spareDqBits & 0x0f ) );
            }

            // Check for ECC spare
            // Note: ECC spares only exist when we have DIMM pairs. We don't
            // want to check the iv_data map for data that won't exist
            // if there is no DIMM on the ECC_SPARE_PORT, so we confirm
            // that a paired DIMM exists here first.
            if ( nullptr != getConnectedDimm(iv_trgt, iv_rank, ECC_SPARE_PORT) )
            {
                uint8_t eccDqBits =
                    iv_data.at(ECC_SPARE_PORT).bitmap[ECC_SPARE_BYTE];
                o_eccSpare = ( 0 == (eccDqBits & 0x0f) );
            }
        }
        else
        {
            o_dramSpare = ( 0 == spareDqBits );
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::setDramSpare( uint8_t i_portSlct, uint8_t i_pins )
{
    #define PRDF_FUNC "[MemDqBitmap::setDramSpare] "

    uint32_t o_rc = SUCCESS;

    do
    {
        // Check to make sure the portSlct is valid
        size_t maxPorts = getNumPorts();
        if ( maxPorts <= i_portSlct )
        {
            PRDF_ERR( PRDF_FUNC "Invalid parameter: i_portSlct=%d", i_portSlct);
            o_rc = FAIL; break;
        }

        uint8_t spareConfig, noSpare, lowNibble, highNibble;
        bool spareSupported = true;
        o_rc = __getSpareInfo( iv_trgt, iv_rank, i_portSlct, spareConfig,
                               noSpare, lowNibble, highNibble, spareSupported );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "__getSpareInfo failed" );
            break;
        }

        if ( noSpare == spareConfig || !spareSupported )
        {
            PRDF_ERR( PRDF_FUNC "DRAM Spare is not available" );
            o_rc = FAIL; break;
        }

        if ( iv_x4Dram )
        {
            i_pins &= 0xf; // limit to 4 bits

            if ( lowNibble == spareConfig )
            {
                i_pins = i_pins << DQS_PER_NIBBLE;
            }
            iv_data[i_portSlct].bitmap[iv_spareByte] |= i_pins;
        }
        else
        {
            i_pins &= 0xff; // limit to 8 bits
            iv_data[i_portSlct].bitmap[iv_spareByte] |= i_pins;
        }

    } while (0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t MemDqBitmap::setEccSpare( uint8_t i_pins )
{
    #define PRDF_FUNC "[MemDqBitmap::setEccSpare] "

    uint32_t o_rc = SUCCESS;

    do
    {
        // Use __getSpareInfo just to check if spares are supported or not
        uint8_t spareConfig, noSpare, lowNibble, highNibble;
        bool spareSupported = true;
        o_rc = __getSpareInfo( iv_trgt, iv_rank, 0, spareConfig,
                               noSpare, lowNibble, highNibble, spareSupported );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "__getSpareInfo failed" );
            break;
        }

        // Break out if spares are not supported
        if ( !spareSupported ) break;

        if ( !iv_x4Dram )
        {
            PRDF_ERR( PRDF_FUNC "0x%08x does not support x4 ECC spare",
                      getHuid(iv_trgt) );
            o_rc = FAIL; break;
        }

        i_pins &= 0xf; // limit to 4 bits
        iv_data[ECC_SPARE_PORT].bitmap[ECC_SPARE_BYTE] |= i_pins;

    } while( 0 );

    return o_rc;

    #undef PRDF_FUNC
}

//##############################################################################
//                              Utility Functions
//##############################################################################

uint32_t setDramInVpd( TargetHandle_t i_trgt, const MemRank & i_rank,
                       MemSymbol i_symbol )
{
    #define PRDF_FUNC "[MemDqBitmap::__setDramInVpd] "

    uint32_t o_rc = SUCCESS;

    do
    {
        MemDqBitmap dqBitmap;
        o_rc = getBadDqBitmap( i_trgt, i_rank, dqBitmap );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getBadDqBitmap(0x%08x, 0x%02x) failed.",
                      getHuid(i_trgt), i_rank.getKey() );
            break;
        }

        o_rc = dqBitmap.setDram( i_symbol );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "setDram() failed." );
            break;
        }

        o_rc = setBadDqBitmap( i_trgt, i_rank, dqBitmap );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "setBadDqBitmap(0x%08x, 0x%02x) failed.",
                      getHuid(i_trgt), i_rank.getKey() );
            break;
        }
    }while(0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

uint32_t clearDramInVpd( TargetHandle_t i_trgt, const MemRank & i_rank,
                         MemSymbol i_symbol )
{
    #define PRDF_FUNC "[MemDqBitmap::__clearDramInVpd] "

    uint32_t o_rc = SUCCESS;

    do
    {
        MemDqBitmap dqBitmap;
        o_rc = getBadDqBitmap( i_trgt, i_rank, dqBitmap );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "getBadDqBitmap(0x%08x, 0x%02x) failed.",
                      getHuid(i_trgt), i_rank.getKey() );
            break;
        }

        o_rc = dqBitmap.clearDram( i_symbol );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "clearDram() failed." );
            break;
        }

        o_rc = setBadDqBitmap( i_trgt, i_rank, dqBitmap );
        if ( SUCCESS != o_rc )
        {
            PRDF_ERR( PRDF_FUNC "setBadDqBitmap(0x%08x, 0x%02x) failed.",
                      getHuid(i_trgt), i_rank.getKey() );
            break;
        }
    }while(0);

    return o_rc;

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

} // end namespace PRDF

