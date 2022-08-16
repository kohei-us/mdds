
AM_CPPFLAGS = -I$(top_srcdir)/include -DNDEBUG

EXTRA_PROGRAMS = \
	mtv-collection \
	basic \
	custom-value-types \
	custom-value-types-custom-store \
	debug-trace \
	element-block1 \
	standard-custom-store \
	event1 \
	pos-hint

mtv_collection_SOURCES = mtv_collection.cpp
basic_SOURCES = basic.cpp
custom_value_types_SOURCES = custom_value_types.cpp
custom_value_types_custom_store_SOURCES = custom_value_types_custom_store.cpp
debug_trace_SOURCES = debug_trace.cpp
element_block1_SOURCES = element_block1.cpp
standard_custom_store_SOURCES = standard_custom_store.cpp
event1_SOURCES = event1.cpp
pos_hint_SOURCES = pos_hint.cpp

TESTS = \
	mtv-collection \
	basic \
	custom-value-types \
	custom-value-types-custom-store \
	debug-trace \
	element-block1 \
	standard-custom-store \
	event1 \
	pos-hint

clean-local:
	rm -f $(TESTS)
