##
## tests/Makefile.am
##
## Process this file with **automake** to produce a Makefile.in
##

#
# This file is part of libunwind.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Test binaries and scripts get installed here
testdir = ${pkglibexecdir}

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/tdep-$(arch) -I$(top_srcdir)/src
AM_CFLAGS = -fno-optimize-sibling-calls

LOG_DRIVER = $(SHELL) $(UNW_TESTDRIVER)

EXTRA_DIST =	run-ia64-test-dyn1 run-ptrace-mapper run-ptrace-misc	\
		run-coredump-unwind \
		run-coredump-unwind-mdi check-namespace.sh.in \
		test-runner.in \
		Gtest-nomalloc.c

CLEANFILES = test-runner
MAINTAINERCLEANFILES = Makefile.in

noinst_PROGRAMS_arch =
noinst_PROGRAMS_cdep =
noinst_PROGRAMS_common =
check_PROGRAMS_arch =
check_PROGRAMS_cdep =
check_PROGRAMS_common = test-proc-info test-static-link \
			test-strerror
check_SCRIPTS_arch =
check_SCRIPTS_cdep =
check_SCRIPTS_common =	check-namespace.sh

XFAIL_TESTS =

if REMOTE_ONLY

perf:

else
 LIBUNWIND_local = $(top_builddir)/src/libunwind.la
if ARCH_IA64
 noinst_PROGRAMS_arch += ia64-test-dyn1
 check_SCRIPTS_arch +=	run-ia64-test-dyn1
 check_PROGRAMS_arch +=	Gia64-test-stack Lia64-test-stack		\
			Gia64-test-nat Lia64-test-nat			\
			Gia64-test-rbs Lia64-test-rbs			\
			Gia64-test-readonly Lia64-test-readonly		\
			ia64-test-setjmp ia64-test-sig
else #!ARCH_IA64
if ARCH_PPC64
if USE_ALTIVEC
 noinst_PROGRAMS_arch += ppc64-test-altivec
endif #USE_ALTIVEC
 check_PROGRAMS_arch += ppc64-test-plt
else  #!ARCH_PPC64
if ARCH_X86_64
 check_PROGRAMS_arch +=	Gx64-test-dwarf-expressions Lx64-test-dwarf-expressions x64-unwind-badjmp-signal-frame
endif #ARCH X86_64
endif #!ARCH_PPC64
endif #!ARCH_IA64

if ARCH_AARCH64
  check_PROGRAMS_arch += Garm64-test-sve-signal Larm64-test-sve-signal   \
                         aarch64-test-plt aarch64-test-frame-record
endif

 check_PROGRAMS_cdep +=	Gtest-bt Ltest-bt \
			Gtest-init Ltest-init				 \
			Gtest-concurrent Ltest-concurrent		 \
			Gtest-trace Ltest-trace				 \
			Ltest-mem-validate				 \
			test-async-sig test-flush-cache test-init-remote \
			test-mem test-reg-state Ltest-varargs		 \
			Ltest-nomalloc Ltest-nocalloc Lrs-race
 noinst_PROGRAMS_cdep += forker Gperf-simple Lperf-simple \
			Gperf-trace Lperf-trace

# unw_init_local2() is not implemented on ia64
if !ARCH_IA64
 check_PROGRAMS_cdep += Ltest-init-local-signal
endif

# Tests that exercise unw_resume, which is only unsupported on some targets
if ENABLE_UNW_RESUME_TESTS
check_PROGRAMS_cdep += Gtest-exc Ltest-exc \
                       Gtest-resume-sig Ltest-resume-sig \
                       Gtest-resume-sig-rt Ltest-resume-sig-rt
endif

if BUILD_PTRACE
 check_SCRIPTS_cdep += run-ptrace-mapper run-ptrace-misc
 check_PROGRAMS_cdep += test-ptrace
 noinst_PROGRAMS_cdep += mapper test-ptrace-misc
if ARCH_X86
 XFAIL_TESTS += test-ptrace
endif
endif

if BUILD_SETJMP
 check_PROGRAMS_cdep += test-setjmp
endif

if SUPPORT_CXX_EXCEPTIONS
 check_PROGRAMS_cdep += Ltest-cxx-exceptions
endif
if ARCH_PPC32
  # https://github.com/libunwind/libunwind/issues/561
  XFAIL_TESTS += Ltest-cxx-exceptions
endif

if ARCH_IA64
 check_PROGRAMS_cdep += Gtest-dyn1 Ltest-dyn1
endif

if OS_LINUX
if BUILD_COREDUMP
 check_SCRIPTS_cdep += run-coredump-unwind
 noinst_PROGRAMS_cdep += crasher test-coredump-unwind

if HAVE_LZMA
 check_SCRIPTS_cdep += run-coredump-unwind-mdi
endif # HAVE_LZMA
endif # BUILD_COREDUMP
endif # OS_LINUX

perf: perf-startup Gperf-simple Lperf-simple Lperf-trace
	@echo "########## Basic performance of generic libunwind:"
	@./Gperf-simple
	@echo "########## Basic performance of local-only libunwind:"
	@./Lperf-simple
	@echo "########## Performance of fast unwind:"
	@./Lperf-trace
	@echo "########## Startup overhead:"
	@$(srcdir)/perf-startup @arch@

endif

check_PROGRAMS = $(check_PROGRAMS_common) $(check_PROGRAMS_cdep) \
		 $(check_PROGRAMS_arch)
check_SCRIPTS =	$(check_SCRIPTS_common) $(check_SCRIPTS_cdep) \
		$(check_SCRIPTS_arch)

TESTS = $(check_PROGRAMS)
if !CROSS_BUILD
 TESTS += $(check_SCRIPTS)
endif

# Use if arch defines but does not support PTRACE_SINGLESTEP
# ptrace request used in the tests.
XFAIL_TESTS_PTRACE_SINGLESTEP = run-ptrace-mapper run-ptrace-misc

if ARCH_MIPS
XFAIL_TESTS += $(XFAIL_TESTS_PTRACE_SINGLESTEP)
endif

if ARCH_RISCV
XFAIL_TESTS += $(XFAIL_TESTS_PTRACE_SINGLESTEP)
endif

if ARCH_ARM
# ARM Linux kernel >=2.6.39 removed PTRACE_SINGLESTEP emulation
XFAIL_TESTS += $(XFAIL_TESTS_PTRACE_SINGLESTEP)
endif

if ARCH_LOONGARCH64
XFAIL_TESTS += $(XFAIL_TESTS_PTRACE_SINGLESTEP)
endif

# This is meant for multilib binaries, -m32.
# ptrace gives EBADREG when testing,
# but generally everything else works.
if XFAIL_PTRACE_TEST
 XFAIL_TESTS += run-ptrace-mapper 
endif

noinst_PROGRAMS = $(noinst_PROGRAMS_common) $(noinst_PROGRAMS_cdep) \
	$(noinst_PROGRAMS_arch)
noinst_HEADERS = ident.h

do_test_subst = sed -e 's,[@]TESTS[@],$(TESTS),g' \
                    -e 's,[@]XFAIL_TESTS[@],$(XFAIL_TESTS),g' \
                    -e 's,[@]LIBDIR[@],$(libdir),g' \
                    -e 's,[@]ARCH[@],$(arch),g'
test-runner: test-runner.in Makefile
	$(AM_V_GEN)$(do_test_subst) <$(srcdir)/test-runner.in >$@
	@chmod +x test-runner

# Make the test binaries installable
test_PROGRAMS = ${check_PROGRAMS} ${noinst_PROGRAMS}
test_SCRIPTS  = ${check_SCRIPTS} test-runner

Lia64_test_readonly_SOURCES = Lia64-test-readonly.c ia64-test-readonly-asm.S
Gia64_test_readonly_SOURCES = Gia64-test-readonly.c ia64-test-readonly-asm.S
Lia64_test_stack_SOURCES = Lia64-test-stack.c ia64-test-stack-asm.S \
				ia64-test-stack.h
Gia64_test_stack_SOURCES = Gia64-test-stack.c ia64-test-stack-asm.S \
				ia64-test-stack.h
Lia64_test_rbs_SOURCES = Lia64-test-rbs.c ia64-test-rbs-asm.S ia64-test-rbs.h
Gia64_test_rbs_SOURCES = Gia64-test-rbs.c ia64-test-rbs-asm.S ia64-test-rbs.h
Lia64_test_nat_SOURCES = Lia64-test-nat.c ia64-test-nat-asm.S
Gia64_test_nat_SOURCES = Gia64-test-nat.c ia64-test-nat-asm.S
ia64_test_dyn1_SOURCES = ia64-test-dyn1.c ia64-dyn-asm.S flush-cache.S \
				flush-cache.h
ppc64_test_altivec_SOURCES = ppc64-test-altivec.c  ppc64-test-altivec-utils.c
ppc64_test_plt_SOURCES = ppc64-test-plt.c


Gx64_test_dwarf_expressions_SOURCES =  Gx64-test-dwarf-expressions.c \
																			 x64-test-dwarf-expressions.S
Lx64_test_dwarf_expressions_SOURCES =  Lx64-test-dwarf-expressions.c \
																			 x64-test-dwarf-expressions.S

Garm64_test_sve_signal_SOURCES = Garm64-test-sve-signal.c
Larm64_test_sve_signal_SOURCES = Larm64-test-sve-signal.c
aarch64_test_plt_SOURCES = aarch64-test-plt.c
aarch64_test_frame_record_SOURCES = aarch64-test-frame-record.c

if COMPILER_SUPPORTS_MARCH_ARMV8_A_SVE
 Garm64_test_sve_signal_CFLAGS = -fno-inline -march=armv8-a+sve
 Larm64_test_sve_signal_CFLAGS = -fno-inline -march=armv8-a+sve
endif
XFAIL_TESTS += Garm64-test-sve-signal Larm64-test-sve-signal

Gtest_init_SOURCES = Gtest-init.cxx
Ltest_init_SOURCES = Ltest-init.cxx
Ltest_cxx_exceptions_SOURCES = Ltest-cxx-exceptions.cxx

Ltest_init_local_signal_SOURCES = Ltest-init-local-signal.c Ltest-init-local-signal-lib.c

x64_unwind_badjmp_signal_frame_SOURCES = x64-unwind-badjmp-signal-frame.c
Gtest_dyn1_SOURCES = Gtest-dyn1.c flush-cache.S flush-cache.h
Ltest_dyn1_SOURCES = Ltest-dyn1.c flush-cache.S flush-cache.h
test_static_link_SOURCES = test-static-link-loc.c test-static-link-gen.c
test_static_link_LDFLAGS = -static
forker_LDFLAGS = -static
Gtest_bt_SOURCES = Gtest-bt.c ident.c
Ltest_bt_SOURCES = Ltest-bt.c ident.c
test_ptrace_misc_SOURCES = test-ptrace-misc.c ident.c
Ltest_nomalloc_SOURCES = Ltest-nomalloc.c
Ltest_nocalloc_SOURCES = Ltest-nocalloc.c
Gtest_trace_SOURCES = Gtest-trace.c ident.c
Ltest_trace_SOURCES = Ltest-trace.c ident.c
Ltest_mem_validate_SOURCES = Ltest-mem-validate.c

LIBUNWIND = $(top_builddir)/src/libunwind-$(arch).la
LIBUNWIND_ptrace = $(top_builddir)/src/libunwind-ptrace.la
LIBUNWIND_coredump = $(top_builddir)/src/libunwind-coredump.la

if USE_ELF32
LIBUNWIND_ELF = $(top_builddir)/src/libunwind-elf32.la
endif
if USE_ELF64
LIBUNWIND_ELF = $(top_builddir)/src/libunwind-elf64.la
endif
if USE_ELFXX
LIBUNWIND_ELF = $(top_builddir)/src/libunwind-elfxx.la
endif

LIBUNWIND_setjmp = $(top_builddir)/src/libunwind-setjmp.la	\
		   $(LIBUNWIND_ELF) $(LIBUNWIND)

test_async_sig_LDADD = $(LIBUNWIND_local) $(PTHREADS_LIB)
test_flush_cache_LDADD = $(LIBUNWIND_local)
test_init_remote_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
test_mem_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
test_reg_state_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
test_ptrace_LDADD = $(LIBUNWIND_ptrace) $(LIBUNWIND)
test_proc_info_LDADD = $(LIBUNWIND)
test_static_link_LDADD = $(LIBUNWIND)
test_strerror_LDADD = $(LIBUNWIND)
Lrs_race_LDADD = $(LIBUNWIND_local) $(PTHREADS_LIB)
Ltest_varargs_LDADD = $(LIBUNWIND_local)
Ltest_init_local_signal_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)

Gtest_bt_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_concurrent_LDADD = $(LIBUNWIND) $(LIBUNWIND_local) $(PTHREADS_LIB)
x64_unwind_badjmp_signal_frame_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_dyn1_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_exc_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_init_LDADD = $(LIBUNWIND) $(LIBUNWIND_local) $(BACKTRACELIB)
Gtest_resume_sig_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_resume_sig_rt_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gperf_simple_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gtest_trace_LDADD=$(LIBUNWIND) $(LIBUNWIND_local)
Gperf_trace_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)

Ltest_bt_LDADD = $(LIBUNWIND_local)
Ltest_concurrent_LDADD = $(LIBUNWIND_local) $(PTHREADS_LIB)
Ltest_cxx_exceptions_LDADD = $(LIBUNWIND_local)
Ltest_dyn1_LDADD = $(LIBUNWIND_local)
Ltest_exc_LDADD = $(LIBUNWIND_local)
Ltest_init_LDADD = $(LIBUNWIND_local)
Ltest_nomalloc_LDADD = $(LIBUNWIND_local) $(DLLIB)
Ltest_nocalloc_LDADD = $(LIBUNWIND_local) $(DLLIB) $(PTHREADS_LIB)
Ltest_resume_sig_LDADD = $(LIBUNWIND_local)
Ltest_resume_sig_rt_LDADD = $(LIBUNWIND_local)
Lperf_simple_LDADD = $(LIBUNWIND_local)
Ltest_trace_LDADD = $(LIBUNWIND_local)
Lperf_trace_LDADD = $(LIBUNWIND_local)
Ltest_mem_validate_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)

test_setjmp_LDADD = $(LIBUNWIND_setjmp)
ia64_test_setjmp_LDADD = $(LIBUNWIND_setjmp)

if BUILD_COREDUMP
test_coredump_unwind_LDADD = $(LIBUNWIND_coredump) $(LIBUNWIND) $(BACKTRACELIB)
endif

Gia64_test_nat_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gia64_test_stack_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gia64_test_rbs_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Gia64_test_readonly_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Lia64_test_nat_LDADD = $(LIBUNWIND_local)
Lia64_test_stack_LDADD = $(LIBUNWIND_local)
Lia64_test_rbs_LDADD = $(LIBUNWIND_local)
Lia64_test_readonly_LDADD = $(LIBUNWIND_local)
ia64_test_dyn1_LDADD = $(LIBUNWIND)
ia64_test_sig_LDADD = $(LIBUNWIND)
ppc64_test_altivec_LDADD = $(LIBUNWIND)
ppc64_test_plt_LDADD = $(LIBUNWIND)

Gx64_test_dwarf_expressions_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Lx64_test_dwarf_expressions_LDADD = $(LIBUNWIND_local)

Garm64_test_sve_signal_LDADD = $(LIBUNWIND) $(LIBUNWIND_local)
Larm64_test_sve_signal_LDADD = $(LIBUNWIND_local)
aarch64_test_plt_LDADD = $(LIBUNWIND)
aarch64_test_frame_record_LDADD = $(LIBUNWIND)
