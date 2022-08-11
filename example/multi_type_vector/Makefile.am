
AM_CPPFLAGS = -I$(top_srcdir)/include -DNDEBUG

EXTRA_PROGRAMS = \
	mtv-collection \
	multi-type-vector \
	multi-type-vector-debug-trace \
	multi-type-vector-element-block1 \
	multi-type-vector-standard-custom-store \
	multi-type-vector-event1 \
	multi-type-vector-pos-hint

mtv_collection_SOURCES = mtv_collection.cpp
multi_type_vector_SOURCES = multi_type_vector.cpp
multi_type_vector_debug_trace_SOURCES = multi_type_vector_debug_trace.cpp
multi_type_vector_element_block1_SOURCES = multi_type_vector_element_block1.cpp
multi_type_vector_standard_custom_store_SOURCES = multi_type_vector_standard_custom_store.cpp
multi_type_vector_event1_SOURCES = multi_type_vector_event1.cpp
multi_type_vector_pos_hint_SOURCES = multi_type_vector_pos_hint.cpp

TESTS = \
	mtv-collection \
	multi-type-vector \
	multi-type-vector-debug-trace \
	multi-type-vector-element-block1 \
	multi-type-vector-standard-custom-store \
	multi-type-vector-event1 \
	multi-type-vector-pos-hint

clean-local:
	rm -f $(TESTS)
