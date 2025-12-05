#!/usr/bin/env sh

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

# m4

reuse annotate \
    --license GPL-3.0-or-later \
    --style python \
    m4/*.m4

# documentation

git ls-tree --full-tree --name-only -r HEAD -- doc/_static \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --skip-unrecognised \
        --merge-copyrights

git ls-tree --full-tree --name-only -r HEAD -- doc \
    | grep -E "\.rst" \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --merge-copyrights

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    --style python \
    doc/conf.py \
    doc/doxygen.conf

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    doc/*.yml \
    doc/*.odg \
    doc/*.txt

# misc

reuse annotate \
    --license "MIT" \
    --copyright "David Tardon" \
        misc/gdb/mdds/__init__.py \
        test/gdb/Makefile.am

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    --merge-copyrights \
    misc/*.cpp

reuse annotate \
    --license "MIT" \
    --copyright "Markus Mohrhard" \
    --merge-copyrights \
    --force-dot-license \
    misc/sorted_string_data.dat \
    misc/mdds.pc.in

# top-level

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    --merge-copyrights \
        .clang-format \
        .gitlab-ci.yml \
        .readthedocs.yaml \
        configure.ac \
        *.md \
        *.sh \
        ./bin/*.sh

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    --merge-copyrights \
    --style python AUTHORS CHANGELOG

# source code

git ls-tree --full-tree --name-only -r HEAD -- example include tools \
    | grep -E "\.(cpp|hpp|inl)$" \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --merge-copyrights \
        --style c

git ls-tree --full-tree --name-only -r HEAD -- \
    test/flat_segment_tree \
    test/include \
    test/multi_type_matrix \
    test/multi_type_vector \
    test/point_quad_tree \
    test/rtree \
    test/segment_tree \
    test/sorted_string_map \
    test/trie_map \
    | grep -E "\.(cpp|hpp|inl)$" \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --merge-copyrights \
        --style c

reuse annotate \
    --license "MIT" \
    --copyright "Kohei Yoshida" \
    --merge-copyrights \
    --style c \
    test/*.cpp

# build scripts

git ls-tree --full-tree --name-only -r HEAD \
    | grep -v "test/gdb" \
    | grep -E "(CMakeLists\.txt|Makefile\.am)" \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --merge-copyrights

# .gitignore and markdown

git ls-tree --full-tree --name-only -r HEAD \
    | grep -E "\.(md|gitignore)$" \
    | xargs \
    reuse annotate \
        --license "MIT" \
        --copyright "Kohei Yoshida" \
        --merge-copyrights

# test/gdb (various copyright holders)

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/default.exp \
    test/gdb/lib/pascal.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2008-2012" \
    --merge-copyrights \
    test/gdb/lib/set_unbuffered_mode.c

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2009-2012" \
    --contributor "Markus Deuling <deuling@de.ibm.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/cell.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2010-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/fortran.exp \
    test/gdb/lib/gdb-python.exp \
    test/gdb/lib/objc.exp \
    test/gdb/lib/prelink-support.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2011-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/boards/native-gdbserver.exp \
    test/gdb/boards/native-stdio-gdbserver.exp \
    test/gdb/lib/prompt.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1993, 1997, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/mips.exp \
    test/gdb/config/mips-idt.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1998, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/dve.exp \
    test/gdb/config/mn10300-eval.exp \
    test/gdb/lib/trace-support.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1997, 1999, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/cfdbug.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2000, 2002-2005, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/gdbserver.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1997 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/d10v.exp \
    test/gdb/config/vxworks.exp \
    test/gdb/config/i960.exp \
    test/gdb/config/m32r.exp \
    test/gdb/config/proelf.exp \
    test/gdb/config/rom68k.exp \
    test/gdb/config/sh.exp \
    test/gdb/config/vr4300.exp \
    test/gdb/config/vr5000.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1995-2000, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/monitor.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1999, 2001, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/sid.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1994, 1997-1998, 2004, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/config/sim.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1993, 1997-1998, 2007-2012" \
    --contributor "Ian Lance Taylor <ian@cygnus.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/config/slite.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1988, 1990-1992, 1994, 1997, 2007-2012" \
    --contributor "Rob Savoye <rob@cygnus.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/config/unix.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1988, 1990-1992, 1995, 1997, 2007-2012" \
    --contributor "Rob Savoye <rob@cygnus.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/config/vx.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1995, 1997, 2007-2012" \
    --contributor "Brendan Kehoe <brendan@cygnus.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/config/vxworks29k.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2004, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/ada.exp \
    test/gdb/lib/future.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2003-2004, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/cp-support.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2010, 2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/dwarf.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2010-2012" \
    --contributor "Ken Werner <ken.werner@de.ibm.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/opencl.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2010-2012" \
    --contributor "Ken Werner <ken.werner@de.ibm.com>" \
    --merge-copyrights \
    test/gdb/lib/cl_util.c \
    test/gdb/lib/cl_util.h \
    test/gdb/lib/opencl_hostapp.c

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1992-2005, 2007-2012" \
    --contributor "Fred Fish <fnf@cygnus.com>" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/gdb.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "2000, 2002-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/gdbserver-support.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1998-2000, 2003-2004, 2006-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/java.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1999-2000, 2002-2005, 2007-2012" \
    --merge-copyrights \
    --style python \
    test/gdb/lib/mi-support.exp

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1995, 1997, 1999, 2003-2004, 2007-2012" \
    --merge-copyrights \
    test/gdb/lib/compiler.c

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "Free Software Foundation, Inc." \
    --year "1995, 1999, 2003-2004, 2007-2012" \
    --merge-copyrights \
    test/gdb/lib/compiler.cc

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "David Tardon" \
    --year 2019 \
    --merge-copyrights \
    --style python \
    test/gdb/config/arm-ice.exp \
    test/gdb/config/bfin.exp \
    test/gdb/config/cygmon.exp \
    test/gdb/config/h8300.exp \
    test/gdb/config/i386-bozo.exp \
    test/gdb/config/m32r-stub.exp \
    test/gdb/lib/pretty-printers.exp \
    test/gdb/mdds/*.exp

reuse annotate \
    --license MIT \
    --copyright "David Tardon" \
    --year 2019 \
    --merge-copyrights \
    --style python \
    test/gdb/src/Makefile.am

reuse annotate \
    --license GPL-3.0-or-later \
    --copyright "David Tardon" \
    --year 2019 \
    --merge-copyrights \
    --style c \
    test/gdb/lib/opencl_kernel.cl \
    test/gdb/src/*.cpp
