#--------------------------------------------------------------------
# Process this file with autoconf to produce a configure script.
#--------------------------------------------------------------------

AC_PREREQ([2.59])
AC_INIT([Chombo], [3.1], [chombousers-request@hpcrdm.lbl.gov], [Chombo.tar.gz], [https://seesar.lbl.gov/anag/chombo/index.html])
AC_CONFIG_SRCDIR([lib/src/BaseTools/REAL.H])

#--------------------------------------------------------------------
# Created files
#--------------------------------------------------------------------

AC_CONFIG_HEADERS([./lib/src/BaseTools/CH_config.H])

#--------------------------------------------------------------------
# System information
#--------------------------------------------------------------------

AC_CANONICAL_HOST

#--------------------------------------------------------------------
# Custom tests use C++
#--------------------------------------------------------------------

AC_PROG_CXX()
AC_LANG([C++])

# Try to determine the make of the compiler.  Many others than gcc set GXX to
# yes so we have to be a bit more clever.
AC_MSG_CHECKING([the make of the C++ compiler])
ch_str_cxxmake=unknown
case $host in
  *86*)
    if test "x`$CXX --version 2>&1 | sed '1d;2{s/.*Free Software Foundation.*/g++/;q;}'`" = "xg++"; then
      ch_str_cxxmake=gcc
    elif test "x`$CXX -V 2>&1 | sed '1{s/.*Intel.*/icpc/;q;}'`" = "xicpc"; then
      ch_str_cxxmake=intel
    elif test "x`$CXX -V 2>&1 | sed '1d;2d;3{s/.*Portland.*/pgCC/;q;}'`" = "xpgCC"; then
      ch_str_cxxmake=portland
    elif test "x$GXX" = "xyes"; then
      # Pathscale will usually come in here.
      ch_str_cxxmake=gcc
    fi
    AC_MSG_RESULT([$ch_str_cxxmake])
    if test "x$ch_str_cxxmake" = "xunknown"; then
      AC_MSG_WARN([Unknown compiler: testing of x86 hardware may fail simply because the right flags are not being used]);
    fi
  ;;
  *    )
    AC_MSG_RESULT([$ch_str_cxxmake])
  ;;
esac

#--------------------------------------------------------------------
# Check for memalign
#--------------------------------------------------------------------

AC_MSG_CHECKING([how to allocate aligned memory])
ch_stat_memalign=unknown

# Check for posix memalign
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#define _XOPEN_SOURCE 600
#include <stdlib.h>]],
[[void* addr;
int m_alignment = 64;
int ier = posix_memalign(&addr, m_alignment, 64);]])],
                  [ch_stat_memalign=posix_memalign])

# Results
AC_MSG_RESULT([$ch_stat_memalign])
if test "x$ch_stat_memalign" = "xposix_memalign"; then
  AC_DEFINE([CHDEF_SYSTEM_HAVE_ALIGNEDALLOC])
  AC_DEFINE([CHDEF_SYSTEM_HAVE_POSIXMEMALIGN])
fi
if test "x$ch_stat_memalign" = "xunknown"; then
  AC_MSG_WARN([No method found for allocating aligned memory - this has
  possible performance implications])
fi

#--------------------------------------------------------------------
# Check for cache line size (unable if cross-compiling)
#--------------------------------------------------------------------

ch_stat_cacheinfo=unknown
ch_int_cachelinesize=unknown
ch_file_cacheinfo=unknown

# Preliminaries
AC_CHECK_FILE([/sys/devices/system/cpu/cpu0/cache/index0])
AC_MSG_CHECKING([cache line size])

# Try system files
if test "x$ac_cv_file__sys_devices_system_cpu_cpu0_cache_index0" = "xyes"; then
  ch_file_cacheinfo=/sys/devices/system/cpu/cpu0/cache
  for d in 0 1; do
    if test -r "$ch_file_cacheinfo/index$d/type"; then
      ch_str_cachetype=`cat $ch_file_cacheinfo/index$d/type`
    fi
    if test "x$ch_str_cachetype" = "xData"; then
      if test -r "$ch_file_cacheinfo/index$d/coherency_line_size"; then
        ch_int_cachelinesize=`cat $ch_file_cacheinfo/index$d/coherency_line_size`
        ch_stat_cacheinfo=file
        break
      fi
    fi
  done
fi

# Try sysconf
if test "x$ch_stat_cacheinfo" = "xunknown"; then
  ch_stat_sysconfcache=no
  AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[#include <unistd.h>
#include <fstream>]],
[[std::ofstream ofile("conftest.data");
ofile << sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
ofile.close();]])],
                [ch_stat_sysconfcache=yes])
  if test "x$ch_stat_sysconfcache" = "xyes" && test -r "conftest.data"; then
    ch_int_cachelinesize=`cat conftest.data`
    ch_stat_cacheinfo=sysconf
  fi
fi

# Results
AC_MSG_RESULT([$ch_int_cachelinesize])
if test "x$ch_stat_cacheinfo" = "xunknown"; then
  AC_MSG_WARN([Unable to find cache line size - using default from
  CH_config.H.in])
else
  AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_CACHELINESIZE], [$ch_int_cachelinesize])
  if test "x$ch_stat_cacheinfo" = "xfile"; then
    AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_HAVE_CACHEINFOFILE], [$ch_file_cacheinfo])
  elif test "x$ch_stat_cacheinfo" = "xsysconf"; then
    AC_DEFINE([CHDEF_SYSTEM_HAVE_CACHESYSCONF])
  fi
fi

#--------------------------------------------------------------------
# Check for x86 vector extensions and intrinsics
#--------------------------------------------------------------------

ch_str_veccpu_feature=unknown
ch_str_veccxx_feature=unknown
ch_int_veccpu_featurebits=0
ch_int_veccxx_featurebits=0
ch_str_sse_h=unknown
ch_str_sse2_h=unknown
ch_str_sse3_h=unknown
ch_str_ssse3_h=unknown
ch_str_sse4d1_h=unknown
ch_str_sse4d2_h=unknown
ch_str_sse4a_h=unknown
ch_str_avx_h=unknown
ch_str_fma_h=unknown
ch_str_avx2_h=unknown
ch_str_intelvec_h=unknown
# Vector operators are tested for sse, sse2, avx, and avx2
ch_stat_vecopp=unknown

AC_MSG_CHECKING([for x86 vector extensions])

case $host_cpu in
  *86*)

#--Test hardware

    ch_stat_cpuid=no
    # Avoid using an optimization flags -- the assembly can cause problems.
    # We will also gradually add sse flags
    ch_str_savecxxflags="$CXXFLAGS"
    CXXFLAGS=""
    AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[#include <fstream>
#include <string>]],
[[unsigned bx, cx, dx;
unsigned volatile res = 0;
std::string str("");
__asm__ __volatile__ ("cpuid" : "=c"(cx), "=d"(dx) : "a"(0x1));
if (dx & (1 << 25)) { res |= (1 << 0); str += "sse"; }
if (dx & (1 << 26)) { res |= (1 << 1); str += " sse2"; }
if (cx & (1 <<  0)) { res |= (1 << 2); str += " sse3"; }
if (cx & (1 <<  9)) { res |= (1 << 3); str += " ssse3"; }
if (cx & (1 << 19)) { res |= (1 << 4); str += " sse4.1"; }
if (cx & (1 << 20)) { res |= (1 << 5); str += " sse4.2"; }
if (cx & (1 << 28)) { res |= (1 << 7); str += " avx1"; }
if (cx & (1 << 12)) { res |= (1 << 8); str += " fma"; }
__asm__ __volatile__ ("cpuid" : "=c"(cx) : "a"(0x80000001));
if (cx & (1 <<  6)) { res |= (1 << 6); str += " sse4a"; }
__asm__ __volatile__ ("cpuid" : "=b"(bx) : "a"(0x7), "c"(0x0));
if (bx & (1 <<  5)) { res |= (1 << 9); str += " avx2"; }
std::ofstream ofile("conftest.data");
ofile << str << " featurebits=" << res;
ofile.close();]])],
                  [ch_stat_cpuid=yes])
    if test "x$ch_stat_cpuid" = "xyes" && test -r "conftest.data"; then
      ch_str_veccpu_feature=`cat conftest.data`
      ch_str_have_cpusse=`expr "x$ch_str_veccpu_feature" : 'x.*\(sse\) '`
      ch_str_have_cpusse2=`expr "x$ch_str_veccpu_feature" : 'x.*\(sse2\) '`
      ch_str_have_cpusse3=`expr "x$ch_str_veccpu_feature" : 'x.* \(sse3\) '`
      ch_str_have_cpussse3=`expr "x$ch_str_veccpu_feature" : 'x.*\(ssse3\) '`
      ch_str_have_cpusse4d1=`expr "x$ch_str_veccpu_feature" : 'x.*\(sse4\.1\) '`
      ch_str_have_cpusse4d2=`expr "x$ch_str_veccpu_feature" : 'x.*\(sse4\.2\) '`
      ch_str_have_cpusse4a=`expr "x$ch_str_veccpu_feature" : 'x.*\(sse4a\) '`
      ch_str_have_cpuavx1=`expr "x$ch_str_veccpu_feature" : 'x.*\(avx1\) '`
      ch_str_have_cpufma=`expr "x$ch_str_veccpu_feature" : 'x.*\(fma\) '`
      ch_str_have_cpuavx2=`expr "x$ch_str_veccpu_feature" : 'x.*\(avx2\) '`
      [ch_int_veccpu_featurebits=`expr "x$ch_str_veccpu_feature" : 'x.*featurebits=\([0-9]*\)'`]
#DBG      echo "$ch_str_have_cpusse x $ch_str_have_cpusse2 x $ch_str_have_cpusse3 x $ch_str_have_cpussse3 x $ch_str_have_cpusse4d1 x $ch_str_have_cpusse4d2 x $ch_str_have_cpusse4a x $ch_str_have_cpuavx1 x $ch_str_have_cpufma x $ch_str_have_cpuavx2 x $ch_int_veccpu_featurebits"
      ch_str_veccpu_feature=`expr "x$ch_str_veccpu_feature" : 'x\(.*\) featurebits=.*'`

#--Test compiler
#--SSE

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse" ;;
      esac
      if test "x$ch_str_have_cpusse" = "xsse"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <xmmintrin.h>  // SSE]],
[[__m128 test_v4sf = _mm_set1_ps(1.f) + _mm_setr_ps(1.f, 2.f, 3.f, 4.f);]])],
                          [ch_str_sse_h="xmmintrin.h"])
        if test "x$ch_str_sse_h" = "xxmmintrin.h"; then
          if test "x$ch_stat_vecopp" != "xno"; then
            ch_stat_vecopp="yes"
          fi
        else
          # Vector ops do not work
          ch_stat_vecopp="no"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <xmmintrin.h>  // SSE]],
[[__m128 test_v4sf = _mm_add_ps(_mm_set1_ps(1.f),
                   _mm_setr_ps(1.f, 2.f, 3.f, 4.f));]])],
                            [ch_str_sse_h="xmmintrin.h"])
        fi
        if test "x$ch_str_sse_h" = "xxmmintrin.h"; then
          ch_str_veccxx_feature="sse"
          ch_int_veccxx_featurebits=1
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128 test_v4sf = _mm_add_ps(_mm_set1_ps(1.f),
                              _mm_setr_ps(1.f, 2.f, 3.f, 4.f));]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse_h $ch_str_intelvec_h"

#--SSE2

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse2" ;;
      esac
      if test "x$ch_str_have_cpusse2" = "xsse2"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <emmintrin.h>  // SSE2]],
[[__m128d test_v2df = _mm_set1_pd(1.) + _mm_setr_pd(1., 2.);]])],
                          [ch_str_sse2_h="emmintrin.h"])
        if test "x$ch_str_sse2_h" = "xemmintrin.h"; then
          if test "x$ch_stat_vecopp" != "xno"; then
            ch_stat_vecopp="yes"
          fi
        else
          # Vector ops do not work
          ch_stat_vecopp="no"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <emmintrin.h>  // SSE2]],
[[__m128d test_v2df = _mm_add_pd(_mm_set1_pd(1.), _mm_setr_pd(1., 2.));]])],
                            [ch_str_sse2_h="emmintrin.h"])
        fi
        if test "x$ch_str_sse2_h" = "xemmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature sse2"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '2'`
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128d test_v2df = _mm_add_pd(_mm_set1_pd(1.), _mm_setr_pd(1., 2.));]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse2_h $ch_str_intelvec_h"

#--SSE3

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse3" ;;
      esac
      if test "x$ch_str_have_cpusse3" = "xsse3"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <pmmintrin.h>  // SSE3]],
[[__m128 test_v4sf = _mm_hadd_ps(_mm_set1_ps(1.f),
                               _mm_setr_ps(1.f, 2.f, 3.f, 4.f));]])],
                          [ch_str_sse3_h="pmmintrin.h"])
        if test "x$ch_str_sse3_h" = "xpmmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature sse3"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '4'`
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128 test_v4sf = _mm_hadd_ps(_mm_set1_ps(1.f),
                               _mm_setr_ps(1.f, 2.f, 3.f, 4.f));]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse3_h $ch_str_intelvec_h"

#--SSSE3

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-mssse3" ;;
      esac
      if test "x$ch_str_have_cpussse3" = "xssse3"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <tmmintrin.h>  // SSSE3]],
[[__m128i test_v4si = _mm_sign_epi32(_mm_set1_epi32(2),
                                   _mm_setr_epi32(1, -1, 1, -1));]])],
                          [ch_str_ssse3_h="tmmintrin.h"])
        if test "x$ch_str_ssse3_h" = "xtmmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature ssse3"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '8'`
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128i test_v4si = _mm_sign_epi32(_mm_set1_epi32(2),
                                   _mm_setr_epi32(1, -1, 1, -1));]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_ssse3_h $ch_str_intelvec_h"

#--SSE4.1

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse4.1" ;;
      esac
      if test "x$ch_str_have_cpusse4d1" = "xsse4.1"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <smmintrin.h>  // SSE4.1]],
[[__m128 test_v4sf = _mm_dp_ps(_mm_set1_ps(1.f),
                             _mm_setr_ps(1.f, 2.f, 3.f, 0.f),
                             113);]])],
                          [ch_str_sse4d1_h="smmintrin.h"])
        if test "x$ch_str_sse4d1_h" = "xsmmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature sse4.1"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '16'`
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128 test_v4sf = _mm_dp_ps(_mm_set1_ps(1.f),
                             _mm_setr_ps(1.f, 2.f, 3.f, 0.f),
                             113);]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse4d1_h $ch_str_intelvec_h"

#--SSE4.2

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse4.2" ;;
      esac
      if test "x$ch_str_have_cpusse4d2" = "xsse4.2"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <nmmintrin.h>  // SSE4.2]],
[[__m128i test_v2di = _mm_cmpgt_epi64(_mm_set1_epi32(1),
                                    _mm_setr_epi32(1, 2, 3, 4));]])],
                          [ch_str_sse4d2_h="nmmintrin.h"])
        if test "x$ch_str_sse4d2_h" = "xnmmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature sse4.2"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '32'`
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // General Vec]],
[[__m128i test_v2di = _mm_cmpgt_epi64(_mm_set1_epi32(1),
                                    _mm_setr_epi32(1, 2, 3, 4));]])],
                            [ch_str_intelvec_h="immintrin.h"],
                            [ch_str_intelvec_h="unknown"])
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse4d2_h $ch_str_intelvec_h"

#--SSE4a

      case $ch_str_cxxmake in
        # Intel likes to pretend that AMD extensions don't exist :P
        intel   ) CXXFLAGS="" ;;
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-msse4a" ;;
      esac
      if test "x$ch_str_have_cpusse4a" = "xsse4a"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <ammintrin.h>  // SSE4a]],
[[__m128i test_v2di = _mm_extracti_si64(_mm_set1_epi32(6), 1, 2);]])],
                          [ch_str_sse4a_h="ammintrin.h"])
        if test "x$ch_str_sse4a_h" = "xammintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature sse4a"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '64'`
          # The general "immintrin.h" doesn't seem to work with SSE4a
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_sse4a_h $ch_str_intelvec_h"

#--AVX

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-mavx" ;;
      esac
      if test "x$ch_str_have_cpuavx1" = "xavx1"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // AVX]],
[[__m256d test_v4df = _mm256_set1_pd(1.) + _mm256_setr_pd(1., 2., 3., 4.);]])],
                          [ch_str_avx_h="immintrin.h"])
        if test "x$ch_str_avx_h" = "ximmintrin.h"; then
          if test "x$ch_stat_vecopp" != "xno"; then
            ch_stat_vecopp="yes"
          fi
        else
          # Vector ops do not work
          ch_stat_vecopp="no"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // AVX]],
[[__m256d test_v4df = _mm256_add_pd(_mm256_set1_pd(1.),
                                  _mm256_setr_pd(1., 2., 3., 4.));]])],
                            [ch_str_avx_h="immintrin.h"])
        fi
        if test "x$ch_str_avx_h" = "ximmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature avx"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '128'`
          # Seems we can only use immintrin.h reliably
          ch_str_intelvec_h="immintrin.h"
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_avx_h $ch_str_intelvec_h"

#--FMA (unvalidated)

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-mfma" ;;  #?? CHECKME
      esac
      if test "x$ch_str_have_cpufma" = "xfma"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // FMA]],  #?? CHECKME
[[__m256d test_v4df = _mm256_fmadd_pd(_mm256_set1_pd(2.), _mm256_set1_pd(3.),
                                    _mm256_setr_pd(1., 2., 3., 4.));]])],
                          [ch_str_fma_h="immintrin.h"])
        if test "x$ch_str_fma_h" = "ximmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature fma"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '256'`
          # Seems we can only use immintrin.h reliably
          ch_str_intelvec_h="immintrin.h"
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_avx_h $ch_str_intelvec_h"

#--AVX2 (unvalidated)

      case $ch_str_cxxmake in
        portland) CXXFLAGS="-Mvect=sse" ;;
        *       ) CXXFLAGS="-mavx2" ;;  #?? CHECKME
      esac
      if test "x$ch_str_have_cpuavx2" = "xavx2"; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // AVX2]],  #?? CHECKME
[[__m256i test_v8si =
  _mm256_set1_epi32(1) + _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8);]])],
                          [ch_str_avx2_h="immintrin.h"])
        if test "x$ch_str_avx2_h" = "ximmintrin.h"; then
          if test "x$ch_stat_vecopp" != "xno"; then
            ch_stat_vecopp="yes"
          fi
        else
          # Vector ops do not work
          ch_stat_vecopp="no"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <immintrin.h>  // AVX2]],  #?? CHECKME
[[__m256i test_v8si = _mm256_add_epi32(
  _mm256_set1_epi32(1), _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8));]])],
                            [ch_str_avx2_h="immintrin.h"])
        fi
        if test "x$ch_str_avx2_h" = "ximmintrin.h"; then
          ch_str_veccxx_feature="$ch_str_veccxx_feature avx2"
          ch_int_veccxx_featurebits=`expr "$ch_int_veccxx_featurebits" + '512'`
          # Seems we can only use immintrin.h reliably
          ch_str_intelvec_h="immintrin.h"
        fi
      fi
#DBG      echo "Values: $ch_str_veccxx_feature $ch_int_veccxx_featurebits $ch_str_avx_h $ch_str_intelvec_h"
    fi

# Reset flags we changed
    CXXFLAGS="$ch_str_savecxxflags"

#--Results

    AC_MSG_RESULT([$ch_str_veccxx_feature])
    if test "$ch_int_veccpu_featurebits" -ne "$ch_int_veccxx_featurebits"; then
      if test "$ch_int_veccxx_featurebits" -eq "0"; then
        AC_MSG_NOTICE([The CPU supports the vector extensions: $ch_str_veccpu_feature, but compiler intrinsics do not work])
      else
        AC_MSG_NOTICE([The CPU supports the vector extensions: $ch_str_veccpu_feature, but compiler intrinsics only work for: $ch_str_veccxx_feature])
      fi
    fi
    if test "$ch_int_veccpu_featurebits" -ne "0"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_CPU_BITS],
                         [$ch_int_veccpu_featurebits])
    fi
    if test "$ch_int_veccxx_featurebits" -ne "0"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_COMPILER_BITS],
                         [$ch_int_veccxx_featurebits])
    fi
    if test "x$ch_str_sse_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE_H], [<$ch_str_sse_h>])
    fi
    if test "x$ch_str_sse2_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE2_H], [<$ch_str_sse2_h>])
    fi
    if test "x$ch_str_sse3_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE3_H], [<$ch_str_sse3_h>])
    fi
    if test "x$ch_str_ssse3_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSSE3_H], [<$ch_str_ssse3_h>])
    fi
    if test "x$ch_str_sse4d1_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE4D1_H], [<$ch_str_sse4d1_h>])
    fi
    if test "x$ch_str_sse4d2_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE4D2_H], [<$ch_str_sse4d2_h>])
    fi
    if test "x$ch_str_sse4a_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_SSE4A_H], [<$ch_str_sse4a_h>])
    fi
    if test "x$ch_str_avx_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_AVX_H], [<$ch_str_avx_h>])
    fi
    if test "x$ch_str_fma_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_FMA_H], [<$ch_str_fma_h>])
    fi
    if test "x$ch_str_avx2_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_AVX2_H], [<$ch_str_avx2_h>])
    fi
    if test "x$ch_str_intelvec_h" != "xunknown"; then
      AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_X86VECEXT_INTEL_H], [<$ch_str_intelvec_h>])
    fi
    if test "x$ch_stat_vecopp" = "xyes"; then
      AC_DEFINE([CHDEF_SYSTEM_X86VECEXT_HAVE_OPERATOR])
    fi
    ;;

  *    )
    AC_MSG_RESULT([not an x86 processor])
    ;;
esac

#--------------------------------------------------------------------
# Check for a hardware popcnt instruction and intrinsic
#--------------------------------------------------------------------

AC_MSG_CHECKING([for hardware popcnt])

ch_int_popcnt_method=0
ch_str_popcnt_func=unknown
ch_str_popcnt_h=unknown

case $host_cpu in
  *86*)

#--Test hardware

    ch_stat_cpuid=no
    # Avoid using an optimization flags -- the assembly can cause problems.
    # We will also gradually add sse flags
    ch_str_savecxxflags="$CXXFLAGS"
    CXXFLAGS=""
    AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[#include <fstream>]],
[[unsigned cx;
unsigned volatile res = 0;
__asm__ __volatile__ ("cpuid" : "=c"(cx) : "a"(0x1));
if (cx & (1 << 23)) { res |= (1 << 0); }
std::ofstream ofile("conftest.data");
ofile << " featurebits=" << res;
ofile.close();]])],
                  [ch_stat_cpuid="yes"])
    if test "x$ch_stat_cpuid" = "xyes" && test -r "conftest.data"; then
      ch_str_popcntcpu_feature=`cat conftest.data`
      [ch_int_popcntcpu_featurebits=`expr "x$ch_str_popcntcpu_feature" : 'x.*featurebits=\([0-9]*\)'`]

#     'popcnt' intrinscis are a bit cumbersome.  With newer versions of gcc, it
#     can be accessed with the _mm_popcnt_u32 intrinsic which is available
#     through popcntintrin.h, in earlier versions through smmintrin.h  It seems
#     easier to just use assembly.

      if test "$ch_int_popcntcpu_featurebits" -eq "1"; then

#--Try assembly

        AC_RUN_IFELSE([AC_LANG_PROGRAM(
[[#include <fstream>]],
[[unsigned x = 0x3;
int ret;
__asm__ ("popcnt %1, %0" : "=r" (ret) : "r" (x));
std::ofstream ofile("conftest.data");  // IO in case of optimization
ofile << ret;
ofile.close();]])],
                      [ch_int_popcnt_method=1])
      fi
    fi

# Reset flags we changed
    CXXFLAGS="$ch_str_savecxxflags"
    ;;
esac

#--Results

if test "$ch_int_popcnt_method" -gt "0"; then
  AC_MSG_RESULT([yes])
  AC_DEFINE_UNQUOTED([CHDEF_SYSTEM_POPCNT_METHOD], [$ch_int_popcnt_method])
else
  AC_MSG_RESULT([no])
fi

#--------------------------------------------------------------------
# Finalize
#--------------------------------------------------------------------

AC_OUTPUT
