## Process this file with automake to produce Makefile.in

SUBDIRS = src

AUTOMAKE_OPTIONS = dejagnu

CLEANFILES = *.log *.sum

RUNTEST = $(RUNTEST_BIN)
AM_RUNTESTFLAGS = BINFILE=src/test GDB=$(GDB)

export PYTHONPATH=$(top_srcdir)/misc/gdb

## vim: set shiftwidth=4 tabstop=4 noexpandtab:
