@echo off
set CXX=cl
set CPPFLAGS=/O2 /Iinclude /W2 /IC:\Users\Kohei\dev\include /DUNIT_TEST /DDEBUG_NODE_BASE /EHsc
%CXX% %CPPFLAGS% src/flat_segment_tree_test.cpp
%CXX% %CPPFLAGS% src/segment_tree_test.cpp
%CXX% %CPPFLAGS% src/rectangle_set_test.cpp
%CXX% %CPPFLAGS% src/point_quad_tree_test.cpp
%CXX% %CPPFLAGS% src/mixed_type_matrix_test.cpp
