// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

/* OpenCL kernel for testing purposes.  */
__kernel void testkernel (__global int *data)
{
  data[get_global_id(0)] = 0x1;
}
