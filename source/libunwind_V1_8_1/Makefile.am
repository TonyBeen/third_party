ACLOCAL_AMFLAGS = -I m4
include_HEADERS = include/libunwind-dynamic.h

if BUILD_PTRACE
include_HEADERS += include/libunwind-ptrace.h
endif BUILD_PTRACE

if BUILD_COREDUMP
include_HEADERS += include/libunwind-coredump.h
endif BUILD_COREDUMP
if BUILD_NTO
include_HEADERS += include/libunwind-nto.h
endif BUILD_NTO
if ARCH_AARCH64
include_HEADERS += include/libunwind-aarch64.h
endif
if ARCH_ARM
include_HEADERS += include/libunwind-arm.h
endif
if ARCH_IA64
include_HEADERS += include/libunwind-ia64.h
endif
if ARCH_HPPA
include_HEADERS += include/libunwind-hppa.h
endif
if ARCH_MIPS
include_HEADERS += include/libunwind-mips.h
endif
if ARCH_X86
include_HEADERS += include/libunwind-x86.h
endif
if ARCH_X86_64
include_HEADERS += include/libunwind-x86_64.h
endif
if ARCH_PPC32
include_HEADERS += include/libunwind-ppc32.h
endif
if ARCH_PPC64
include_HEADERS += include/libunwind-ppc64.h
endif
if ARCH_SH
include_HEADERS += include/libunwind-sh.h
endif
if ARCH_S390X
include_HEADERS += include/libunwind-s390x.h
endif
if ARCH_RISCV
include_HEADERS += include/libunwind-riscv.h
endif
if ARCH_LOONGARCH64
include_HEADERS += include/libunwind-loongarch64.h
endif

if !REMOTE_ONLY
include_HEADERS += include/libunwind.h
if BUILD_UNWIND_HEADER
include_HEADERS += include/unwind.h
endif
endif

# TODO 修改头文件安装位置
includedir = $(prefix)/include/libunwind

nodist_include_HEADERS = include/libunwind-common.h

SUBDIRS = src

if CONFIG_TESTS
SUBDIRS += tests
endif

if CONFIG_DOCS
SUBDIRS += doc
endif

noinst_HEADERS = include/dwarf.h include/dwarf_i.h include/dwarf-eh.h	\
	include/compiler.h include/libunwind_i.h include/mempool.h	\
	include/remote.h						\
	include/tdep-aarch64/dwarf-config.h				\
	include/tdep-aarch64/jmpbuf.h					\
	include/tdep-aarch64/libunwind_i.h				\
	include/tdep-arm/dwarf-config.h	include/tdep-arm/ex_tables.h	\
	include/tdep-arm/jmpbuf.h include/tdep-arm/libunwind_i.h	\
	include/tdep-ia64/jmpbuf.h include/tdep-ia64/rse.h 		\
	include/tdep-ia64/libunwind_i.h	include/tdep-ia64/script.h	\
	include/tdep-hppa/libunwind_i.h					\
	include/tdep-hppa/jmpbuf.h include/tdep-hppa/dwarf-config.h	\
	include/tdep-mips/libunwind_i.h					\
	include/tdep-mips/jmpbuf.h include/tdep-mips/dwarf-config.h	\
	include/tdep-x86/libunwind_i.h					\
	include/tdep-x86/jmpbuf.h include/tdep-x86/dwarf-config.h	\
	include/tdep-x86_64/libunwind_i.h				\
	include/tdep-x86_64/jmpbuf.h include/tdep-x86_64/dwarf-config.h \
	include/tdep-ppc32/dwarf-config.h				\
	include/tdep-ppc32/jmpbuf.h include/tdep-ppc32/libunwind_i.h	\
	include/tdep-ppc64/dwarf-config.h				\
	include/tdep-ppc64/jmpbuf.h include/tdep-ppc64/libunwind_i.h	\
	include/tdep-sh/dwarf-config.h					\
	include/tdep-sh/jmpbuf.h include/tdep-sh/libunwind_i.h		\
	include/tdep-s390x/dwarf-config.h				\
	include/tdep-s390x/jmpbuf.h include/tdep-s390x/libunwind_i.h	\
	include/tdep-riscv/dwarf-config.h				\
	include/tdep-riscv/jmpbuf.h include/tdep-riscv/libunwind_i.h	\
	include/tdep-loongarch64/dwarf-config.h				\
	include/tdep-loongarch64/jmpbuf.h				\
	include/tdep-loongarch64/libunwind_i.h				\
	include/tdep/libunwind_i.h					\
	include/tdep/jmpbuf.h include/tdep/dwarf-config.h

EXTRA_DIST = include/libunwind-common.h.in

MAINTAINERCLEANFILES = \
	Makefile.in \
	INSTALL \
	aclocal.m4 \
	configure \
	config/compile \
	config/config.guess \
	config/config.sub \
	config/depcomp \
	config/install-sh \
	config/ltmain.sh \
	config/missing \
	include/config.h.in \
	include/config.h.in~

