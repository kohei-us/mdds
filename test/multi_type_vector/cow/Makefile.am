# SPDX-FileCopyrightText: 2026 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-soa

test_soa_SOURCES = \
	test_soa.cpp \
	$(top_srcdir)/test/test_global.cpp

test_soa_CPPFLAGS = \
	-I$(srcdir) \
	-I$(srcdir)/tc \
	$(AM_CPPFLAGS)

EXTRA_DIST = \
	common_types.hpp \
	tc/share.hpp \
	tc/detach.hpp \
	tc/lifetime.hpp \
	tc/noncopyable.hpp \
	tc/run.hpp

TESTS = test-soa

@VALGRIND_CHECK_RULES@
