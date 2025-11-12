
AM_CPPFLAGS = -I$(top_srcdir)/include -DNDEBUG

EXTRA_PROGRAMS = \
	mtv-collection \
	basic \
	clone \
	custom-value-types \
	custom-value-types-custom-store \
	debug-trace \
	element-block1 \
	exec-policy \
	standard-custom-store \
	event1 \
	pos-hint

mtv_collection_SOURCES = mtv_collection.cpp
basic_SOURCES = basic.cpp
clone_SOURCES = clone.cpp
custom_value_types_SOURCES = custom_value_types.cpp
custom_value_types_custom_store_SOURCES = custom_value_types_custom_store.cpp
debug_trace_SOURCES = debug_trace.cpp
element_block1_SOURCES = element_block1.cpp

exec_policy_SOURCES = exec_policy.cpp
exec_policy_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(TBB_CFLAGS)
exec_policy_LDADD = $(TBB_LIBS)

standard_custom_store_SOURCES = standard_custom_store.cpp
event1_SOURCES = event1.cpp
pos_hint_SOURCES = pos_hint.cpp

TESTS = \
	mtv-collection \
	basic \
	clone \
	custom-value-types \
	custom-value-types-custom-store \
	debug-trace \
	element-block1 \
	exec-policy \
	standard-custom-store \
	event1 \
	pos-hint

clean-local:
	rm -f $(TESTS)
