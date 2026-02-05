/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include <mdds/version.hpp>

#include <string>

void test_version()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        std::string ver = MDDS_VERSION; // it's a string literal
        std::cout << "version: " << ver << std::endl;

        std::ostringstream os;
        os << MDDS_VERSION_MAJOR << '.' << MDDS_VERSION_MINOR << '.' << MDDS_VERSION_MICRO;
        assert(ver == os.str());
    }

    {
        // make sure these are integer literals
        int ver = MDDS_VERSION_MAJOR;
        std::cout << "major: " << ver << std::endl;
        ver = MDDS_VERSION_MINOR;
        std::cout << "minor: " << ver << std::endl;
        ver = MDDS_VERSION_MICRO;
        std::cout << "micro: " << ver << std::endl;
    }

    {
        std::string ver = MDDS_API_VERSION; // it's a string literal
        std::cout << "API version: " << ver << std::endl;

        std::ostringstream os;
        os << MDDS_API_VERSION_MAJOR << '.' << MDDS_API_VERSION_MINOR;
        assert(ver == os.str());
    }

    {
        // make sure these are integer literals
        int ver = MDDS_API_VERSION_MAJOR;
        std::cout << "API major: " << ver << std::endl;
        ver = MDDS_API_VERSION_MINOR;
        std::cout << "API minor: " << ver << std::endl;
    }
}

int main()
{
    test_version();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
