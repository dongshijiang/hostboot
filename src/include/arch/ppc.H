/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/arch/ppc.H $                                      */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
/* [+] Google Inc.                                                        */
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
#ifndef __KERNEL_PPCARCH_H
#define __KERNEL_PPCARCH_H

#include <kernel/types.h>
#include <builtins.h>

ALWAYS_INLINE
inline uint64_t getSRR0()
{
    register uint64_t srr0 = 0;
    asm volatile("mfsrr0 %0" : "=r" (srr0));
    return srr0;
}

ALWAYS_INLINE
inline uint64_t getSRR1()
{
    register uint64_t srr1 = 0;
    asm volatile("mfsrr1 %0" : "=r" (srr1));
    return srr1;
}

ALWAYS_INLINE
inline void setSRR0(uint64_t _srr0)
{
    register uint64_t srr0 = _srr0;
    asm volatile("mtsrr0 %0" : : "r" (srr0));
}

ALWAYS_INLINE
inline void setSRR1(uint64_t _srr1)
{
    register uint64_t srr1 = _srr1;
    asm volatile("mtsrr1 %0" : : "r" (srr1));
}

ALWAYS_INLINE
inline uint64_t getHSRR0()
{
    register uint64_t hsrr0 = 0;
    asm volatile("mfspr %0, 314" : "=r" (hsrr0));
    return hsrr0;
}

ALWAYS_INLINE
inline uint64_t getHSRR1()
{
    register uint64_t hsrr1 = 0;
    asm volatile("mfspr %0, 315" : "=r" (hsrr1));
    return hsrr1;
}

ALWAYS_INLINE
inline void setHSRR0(uint64_t _hsrr0)
{
    register uint64_t hsrr0 = _hsrr0;
    asm volatile("mtspr 314, %0" : : "r" (hsrr0));
}

ALWAYS_INLINE
inline void setHSRR1(uint64_t _hsrr1)
{
    register uint64_t hsrr1 = _hsrr1;
    asm volatile("mtspr 315, %0" : : "r" (hsrr1));
}

ALWAYS_INLINE
inline uint64_t getPVR()
{
    register uint64_t pvr = 0;
    asm volatile("mfspr %0, 287" : "=r" (pvr));
    return pvr;
}

ALWAYS_INLINE
inline uint64_t getPIR()
{
    register uint64_t pir = 0;
    asm volatile("mfspr %0, 1023" : "=r" (pir));
    return pir;
}

ALWAYS_INLINE
inline uint64_t getSPRG2()
{
    register uint64_t sprg2 = 0;
    asm volatile("mfsprg2 %0" : "=r" (sprg2));
    return sprg2;
}

ALWAYS_INLINE
inline void setSPRG2(uint64_t _sprg2)
{
    register uint64_t sprg2 = _sprg2;
    asm volatile("mtsprg2 %0" : : "r" (sprg2));
    return;
}


ALWAYS_INLINE
inline uint64_t getSPRG3()
{
    register uint64_t sprg3 = 0;
    asm volatile("mfsprg3 %0" : "=r" (sprg3));
    return sprg3;
}

ALWAYS_INLINE
inline void setSPRG3(uint64_t _sprg3)
{
    register uint64_t sprg3 = _sprg3;
    asm volatile("mtsprg3 %0" : : "r" (sprg3));
    return;
}

ALWAYS_INLINE
inline uint64_t getMSR()
{
    register uint64_t msr = 0;
    asm volatile("mfmsr %0" : "=r" (msr));
    return msr;
}

ALWAYS_INLINE
inline void setMSR(uint64_t _msr)
{
    register uint64_t msr = _msr;
    asm volatile("mtmsr %0; isync" :: "r" (msr));
}

ALWAYS_INLINE
inline uint64_t getDSISR()
{
    register uint64_t dsisr = 0;
    asm volatile("mfspr %0, 18" : "=r" (dsisr));
    return dsisr;
}

ALWAYS_INLINE
inline uint64_t getDAR()
{
    register uint64_t dar = 0;
    asm volatile("mfspr %0, 19" : "=r" (dar));
    return dar;
}

ALWAYS_INLINE
inline uint64_t getTB()
{
    register uint64_t tb = 0;
    asm volatile("mfspr %0, 268" : "=r" (tb));
    return tb;
}

ALWAYS_INLINE
inline void setTB(uint64_t _tb)
{
    // The bottom 24 bits of the timebase can't be written so round it up.
    register uint64_t tb = (_tb + 0x1000000);
    // Note that SPR 286 is intended (while getTB is 268) here.
    // This is the mttbu40 instruction and not the mttb, which doesn't exist.
    asm volatile("mtspr 286, %0" :: "r" (tb));
}

ALWAYS_INLINE
inline void setDEC(uint64_t _dec)
{
    register uint64_t dec = _dec;
    asm volatile("mtdec %0" :: "r" (dec));
}

ALWAYS_INLINE
inline void setRPR(uint64_t _rpr)
{
    register uint64_t rpr = _rpr;
    asm volatile("mtspr 186, %0" :: "r"(rpr));
}

ALWAYS_INLINE
inline void sync()
{
    asm volatile("sync" ::: "memory");
}

ALWAYS_INLINE
inline void lwsync()
{
    asm volatile("lwsync" ::: "memory");
}

ALWAYS_INLINE
inline void isync()
{
    asm volatile("isync" ::: "memory");
}

ALWAYS_INLINE
inline void eieio()
{
    asm volatile("eieio" ::: "memory");
}

ALWAYS_INLINE
inline void msgsync()
{
    // See POWER ISA 5.9.2 for details
    //asm volatile("msgsync" ::: "memory");
    asm volatile(".long 0x7C0006EC"); // Our GCC doesn't support 'msgsync' yet
    //  [011111 ///// ///// ///// 11011 10110/]

    // There is a P9 DD2 workaround that a lwsync is also required
    //  after a msgsync
    asm volatile("lwsync" ::: "memory");
}


ALWAYS_INLINE
inline uint64_t getHMER()
{
    register uint64_t hmer = 0;
    asm volatile("mfspr %0, 336" : "=r" (hmer));
    return hmer;
}

ALWAYS_INLINE
inline void setHMER(uint64_t _hmer)
{
    register uint64_t hmer = _hmer;
    asm volatile("mtspr 336, %0" : : "r" (hmer));
    return;
}

ALWAYS_INLINE
inline uint64_t getHEIR()
{
    register uint64_t heir = 0;
    asm volatile("mfspr %0, 339" : "=r" (heir));
    return heir;
}

ALWAYS_INLINE
inline uint64_t getLPCR()
{
    register uint64_t lpcr = 0;
    asm volatile("mfspr %0, 318" : "=r" (lpcr));
    return lpcr;
}

ALWAYS_INLINE
inline void setLPCR(uint64_t _lpcr)
{
    register uint64_t lpcr = _lpcr;
    asm volatile("mtspr 318, %0; isync" :: "r" (lpcr));
}

ALWAYS_INLINE
inline uint64_t getHRMOR()
{
    register uint64_t hrmor = 0;
    asm volatile("mfspr %0, 313" : "=r" (hrmor));
    return hrmor;
}

ALWAYS_INLINE
inline uint64_t getURMOR()
{
    register uint64_t urmor = 0;
    asm volatile("mfspr %0, 505" : "=r" (urmor));
    return urmor;
}

ALWAYS_INLINE
inline uint64_t getPTCR()
{
    register uint64_t ptcr = 0;
    asm volatile("mfspr %0, 464" : "=r" (ptcr));
    return ptcr;
}

ALWAYS_INLINE
inline void setPTCR(uint64_t _ptcr)
{
    register uint64_t ptcr = _ptcr;
    asm volatile("mtspr 464, %0; isync" :: "r" (ptcr));
}

ALWAYS_INLINE
inline void setThreadPriorityLow()
{
    asm volatile("or 1,1,1");
}

ALWAYS_INLINE
inline void setThreadPriorityHigh()
{
    asm volatile("or 2,2,2");
}

ALWAYS_INLINE
inline void setThreadPriorityVeryHigh()
{
    asm volatile("or 7,7,7");
}


ALWAYS_INLINE
inline void dcbf(void* _ptr)
{
    register void* ptr = _ptr;
    asm volatile("dcbf 0, %0" : : "b" (ptr) : "memory");
}

ALWAYS_INLINE
inline void dcbst(void* _ptr)
{
    register void* ptr = _ptr;
    asm volatile("dcbst 0, %0" : : "b" (ptr) : "memory");
}

ALWAYS_INLINE
inline void dcbz(void* _ptr)
{
    register void* ptr = _ptr;
    asm volatile("dcbz 0, %0" : : "b" (ptr) : "memory");
}

ALWAYS_INLINE
inline void icbi(void* _ptr)
{
    register void* ptr = _ptr;
    asm volatile("icbi 0, %0" : : "b" (ptr) : "memory");
}

ALWAYS_INLINE
inline void nap()
{
    asm volatile(".long 0x4C0002E4"); // When GCC supports 'stop', use it
}

ALWAYS_INLINE
inline void setPSSCR(uint64_t _psscr)
{
    register uint64_t psscr = _psscr;
    asm volatile("mtspr 855, %0; isync" :: "r" (psscr));
}

ALWAYS_INLINE
inline uint64_t getPSSCR()
{
    register uint64_t psscr = 0;
    asm volatile("mfspr %0, 855" : "=r" (psscr));
    return psscr;
}

ALWAYS_INLINE
inline uint64_t getHID()
{
    register uint64_t hid = 0;
    asm volatile("mfspr %0, 1008" : "=r" (hid));
    return hid;
}

ALWAYS_INLINE
inline void setHID(uint64_t _hid)
{
    register uint64_t hid = _hid;
    asm volatile("mtspr 1008, %0; isync" :: "r" (hid));
}

ALWAYS_INLINE
inline size_t getCacheLineBytes()
{
    return 128;
}

ALWAYS_INLINE
inline size_t getCacheLineWords()
{
    return getCacheLineBytes() / sizeof(uint64_t);
}

ALWAYS_INLINE
inline void writeScratchReg(uint64_t _scratch_addr, uint64_t _data)
{
    register uint64_t scratch_addr = _scratch_addr;
    register uint64_t data = _data;

    asm volatile("mtspr 276, %0\n"
                 "isync\n"
                 "mtspr 277, %1"
                 :: "r" (scratch_addr), "r" (data));

}

#ifdef __HOSTBOOT_RUNTIME

/** @brief  getDarn - deliver a random number instruction
 *  Returns 64 bits of random data, requires random number generator
 *  configured appropriately + locked down, only available at runtime.
 */
ALWAYS_INLINE
inline uint64_t getDarn()
{
    register uint64_t rt = 0;
    asm volatile(".long 0x7C0105E6 | "
                 "((%0 & 0x1F) << 21)" :
                 "=r" (rt));
    return rt;
}

#endif

/** @brief  This is a special assembler instruction that is a nop on
 *  regular hardware, but has special meaning to Simics.  Code that
 *  executes this instruction in Simics will cause a "hap," a
 *  Simics term.  If there is no hap handler registered, and magic
 *  breakpoints have not been enabled with
 *      simics> enable-magic-breakpoint
 *  then this instruction is also a nop in Simics.
 *
 *  If magic breakpoints are enabled, and there is no hap handler, then
 *  when Hostboot code executes this instruction in Simics, Simics will
 *  stop the simulation. (Prompt changes from running> to simics> )
 *
 *  If a hap is registered, then Simics will call the hap handler.  Hap
 *  handlers are written in Python, and the best place for them is
 *
 *     src/build/debug/simics-debug-framework.py
 *
 *  Sample code to register the hap handler:
 *        # arg contains the integer parameter n passed to MAGIC_INSTRUCTION(n)
 *        def magic_instruction_callback(user_arg, cpu, arg):
 *            # print to console works...
 *            print "Hit magic instruction ", arg
 *            # Or else stop the simulation...
 *            SIM_break_simulation( "Stopped at magic instruction" )
 *
 *        # Register the magic instruction callback.
 *        SIM_hap_add_callback( "Core_Magic_Instruction", magic_instruction_callback, None )
 *
 *        # Better to register the Hostboot range 7000-7999
 *        # so that PHYP and others won't be affected.
 *        SIM_hap_add_callback_range( "Core_Magic_Instruction", magic_instruction_callback, None, 7000, 7999 )
 *
 *  The argument n is an integer from 0..8191 which Simics passes to the hap
 *  handler in parameter 3, or "arg" in the sample code above.
 */
ALWAYS_INLINE
inline void MAGIC_INSTRUCTION(int _n)
{
    register int n = _n;
    isync();
    long register r3 asm("r3");
    asm volatile("rlwimi %1,%1,0,%2,%3"         \
        : "=r"(r3) : "i" (((n) >> 8) & 0x1f),   \
        "i" (((n) >> 4) & 0xf),                 \
        "i" ((((n) >> 0) & 0xf) | 16),          \
        "r"(r3));                               \
}

// Simics components that we can raise log levels for
// It is important to keep these lined up with
// simics-debug-framework.py
constexpr uint8_t PIB_PSU              = 0;
constexpr uint8_t SBE_INT_BO           = 1;

constexpr uint8_t ENABLE_OUTPUT  = 1;
constexpr uint8_t DISABLE_OUTPUT = 0;

constexpr uint8_t LOG_LEVEL_OFF  = 0;
constexpr uint8_t LOG_LEVEL_LOW  = 1;
constexpr uint8_t LOG_LEVEL_MED  = 2;
constexpr uint8_t LOG_LEVEL_HIGH = 3;
constexpr uint8_t LOG_LEVEL_MAX  = 4;

// Arguments to MAGIC_INSTRUCTION().
// To ensure they do not conflict with haps from other groups (PHYP
// for example), assign hap numbers in the range 7000..7999 (decimal).
// Presently, the hap handler for magic instruction is found in
// src/build/debug/simics-debug-framework.py
enum
{
    MAGIC_SIMICS_CORESTATESAVE = 10, // Indicate to the PHYP model of simics
                                     // that we are preparing to wake up a core
                                     // or thread.  This allows them to save
                                     // some state from the core doing the
                                     // wakeup to apply into the woken one.

    MAGIC_SIMICS_FUSEDCOREWAKE = 11, // Indicate to the PHYP model of simics
                                     // that we are waking up and expecting
                                     // CORES to become fused.

    // 7000-7999 are defined by Hostboot
    MAGIC_SHUTDOWN            =  7006, // KernelMisc::shutdown() called.
    MAGIC_BREAK               =  7007, // hard-code a breakpoint
    MAGIC_RANDOM              =  7008, // generate random number
    MAGIC_MEMORYLEAK_FUNCTION =  7009, // A memory was function called.
    MAGIC_FAKEPAYLOAD_ENTER   =  7010, // Entered the fake payload.
    MAGIC_SIMICS_CHECK        =  7011, // Check if system is running on simics
    MAGIC_LOAD_PAYLOAD        =  7012, // load payload from flash
    MAGIC_HB_DUMP             =  7014, // Create a hostboot dump
    MAGIC_BREAK_ON_ERROR      =  7018, // Breakpoint in error cases if
                                       //  env var HB_BREAK_ON_ERROR
    MAGIC_GET_SBE_TRACES      =  7019, // Collect SBE traces
    MAGIC_PRINT_ISTEP         =  7020, // Print istep to simics console
    MAGIC_PRINT_TWO_REGS      =  7021, // Print 2 numbers passed in
    MAGIC_SET_LOG_LEVEL       =  7022, // Set log level for given component
    MAGIC_TOGGLE_OUTPUT       =  7023, // Enable simic log capture

    MAGIC_CONTINUOUS_TRACE    =  7055, // extract mixed trace buffer
    MAGIC_GCOV_MODULE_UNLOAD  =  7056, // extract gcov info


    // 8000-8999 are defined by the Simics CEC team
    MAGIC_SIMICS_FSP_WAIT      =  8001, // Notify we are/aren't waiting for FSP
                                        //   1=waiting, 2=done waiting
    MAGIC_SIMICS_SHUTDOWN      =  8006, // Notify we are shutting down
    MAGIC_SIMICS_ISTEP         =  8020, // Log istep, same parms as 7020
    MAGIC_SIMICS_EXIT_CACHE_CONTAINED = 8021, // Exiting cache contained mode
};

/**
 * @brief Collect SBE traces via external debug commands
 * @param[in]  Processor number
 * @param[in]  SBE Return Code
 */
#define MAGIC_INST_GET_SBE_TRACES(_procnum,_rc) \
    asm volatile("mr 4, %0; mr 5, %1" :: \
                 "r" (_procnum), "r" (_rc) : "4", "5"); \
    MAGIC_INSTRUCTION(MAGIC_GET_SBE_TRACES); \

/**
 * @brief Display istep numbers on the simics console
 * @param[in]  Major istep number
 * @param[in]  Minor istep number
 */
#define MAGIC_INST_PRINT_ISTEP(_major,_minor) \
    asm volatile("mr 4, %0; mr 5, %1" :: \
                 "r" (_major), "r" (_minor) : "4", "5"); \
    MAGIC_INSTRUCTION(MAGIC_PRINT_ISTEP); \
    MAGIC_INSTRUCTION(MAGIC_SIMICS_ISTEP); \

/**
 * @brief Display 2 numbers on the simics console
 * @param[in]  First number (uint64_t)
 * @param[in]  Second number (uint64_t)
 */
#define MAGIC_INST_PRINT_2_REGS(_first, _second) \
    asm volatile("mr 4, %0; mr 5, %1" :: \
                 "r" (_first), "r" (_second) : "4", "5"); \
    MAGIC_INSTRUCTION(MAGIC_PRINT_TWO_REGS); \

/**
 * @brief Notify simics that we are waiting for a FSP message
 */
#define MAGIC_WAITING_FOR_FSP() \
    asm volatile("mr 4, %0" :: "r" (1) : "4"); \
    MAGIC_INSTRUCTION(MAGIC_SIMICS_FSP_WAIT); \

/**
 * @brief Notify simics that we are done waiting for a FSP message
 */
#define MAGIC_DONE_WAITING_FOR_FSP() \
    asm volatile("mr 4, %0" :: "r" (2) : "4"); \
    MAGIC_INSTRUCTION(MAGIC_SIMICS_FSP_WAIT); \
/**
 * @brief set the log level for a given component
 * @param[in]  _componentId Enum of simics comp ids
 * @param[in]  _logLevel desired log level
 */
#define MAGIC_INST_SET_LOG_LEVEL(_componentId, _logLevel) \
    asm volatile("mr 4, %0; mr 5, %1" :: \
                 "r" (_componentId), "r" (_logLevel) : \
                 "4", "5"); \
    MAGIC_INSTRUCTION(MAGIC_SET_LOG_LEVEL); \

/**
 * @brief Toggle simics log collections on or off, if one
 *        all simics logs will be collected in <sandbox>/simics/hb_simics_log.txt
 * @param[in]  1 > will enable log collections, 0 stops collection
 */
#define MAGIC_INST_TOGGLE_OUTPUT(_enable) \
    asm volatile("mr 4, %0" :: \
                 "r" (_enable) : \
                 "4"); \
    MAGIC_INSTRUCTION(MAGIC_TOGGLE_OUTPUT); \

#endif
