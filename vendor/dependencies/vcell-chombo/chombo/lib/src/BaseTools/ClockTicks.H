#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#ifndef _CLOCKTICKS_H_
#define _CLOCKTICKS_H_

// (dfm 4/29/08)note that NamespaceHeader.H gets included 3x, once for
// each "if"...


#if defined(__INTEL_COMPILER) && defined(__ia64__)
#include <ia64intrin.h>
#include <ia64regs.h>
#include "BaseNamespaceHeader.H"

inline unsigned long long int ch_ticks()
{
  volatile unsigned long long int rtn = __getReg(_IA64_REG_AR_ITC);
  return rtn;
}
#define CH_TICKS

#elif defined(__x86_64__) || defined(__i386__) || defined(__i686__)
#include "BaseNamespaceHeader.H"

union clockunion
{
  unsigned int i[2];
  unsigned long long int l;
};

inline unsigned long long int ch_ticks()
{
  volatile clockunion ret;
  __asm__ __volatile__ ("rdtsc" : "=a"(ret.i[0]), "=d"(ret.i[1]));
  return ret.l;

  // for some reason this form executes on 64, bit acts funny.
  //  unsigned long long int rtn;
  //  __asm__ __volatile__("rdtsc" : "=A"(rtn));
  //  return rtn;
}
#define CH_TICKS

#elif defined(_POWER) || defined(_POWERPC) || defined(__powerpc__)
#include "BaseNamespaceHeader.H"

inline unsigned long long int ch_ticks()
{
  volatile unsigned long long int tbr;
  __asm__ __volatile__("mftb %[tbr]" : [tbr] "=r" (tbr):);
  return tbr;
}
#define CH_TICKS
#elif defined(__aarch64__) || defined(__arm64__)
#include "BaseNamespaceHeader.H"

// AArch64 predates neither this file nor Apple Silicon by much, but Chombo was
// written before either. Without a branch here the #else below defines no
// ch_ticks at all, and CH_Timer.H calls it unguarded from TraceTimer::leafStart
// and leafStop -- so every translation unit that pulls in the timer fails to
// compile on arm64.
//
// CNTVCT_EL0 is the virtual counter, readable from user space on both Linux and
// macOS. Same raw-counter semantics as the x86 rdtsc branch above; TraceTimer
// only ever uses differences, so the tick rate does not need to be known.
inline unsigned long long int ch_ticks()
{
  unsigned long long int cntvct;
  __asm__ __volatile__("mrs %0, cntvct_el0" : "=r" (cntvct));
  return cntvct;
}
#define CH_TICKS
#else
// if we fall all the way through to here, we don't really do anything,
// but we still need the NamespaceHeader to balance
// the namespacefooter. (DFM 4/28/09)
#include "BaseNamespaceHeader.H"
#endif


#include "BaseNamespaceFooter.H"
#endif
