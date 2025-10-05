AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include

TARGETS = \
	mtv-copy-blocks \
	mtv-clone-noncopyable

EXTRA_PROGRAMS = \
	$(TARGETS)

CLEANFILES = \
	$(TARGETS)

mtv_copy_blocks_SOURCES = \
	mtv_copy_blocks.cpp \
	$(top_srcdir)/test/test_global.cpp

mtv_copy_blocks_LDADD = -ltbb

mtv_clone_noncopyable_SOURCES = \
	mtv_clone_noncopyable.cpp \
	$(top_srcdir)/test/test_global.cpp

mtv_clone_noncopyable_LDADD = -ltbb
