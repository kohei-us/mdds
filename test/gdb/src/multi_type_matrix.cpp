// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <mdds/multi_type_matrix.hpp>

void stop();

using mdds::multi_type_matrix;

void test_multi_type_matrix()
{
    typedef mdds::multi_type_matrix<mdds::mtm::std_string_traits> mtm_type;

    mtm_type empty_mtm;

    mtm_type mtm(4, 2);
    mtm.set(1, 0, 1);
    mtm.set(2, 0, 2);
    mtm.set(3, 0, std::string("a"));
    mtm.set(0, 1, std::string("b"));
    mtm.set(1, 1, true);
    mtm.set(3, 1, 0.5);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
