# -*- Mode: Python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
# Copyright (c) 2019 David Tardon
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

import gdb
import gdb.printing

import six

class IntrusivePtr(object):

    def __init__(self, val):
        self.val = val

    def get(self):
        return self.val['px']

    def __eq__(self, other):
        if isinstance(other, IntrusivePtr):
            return self.get() == other.get()
        return False

    def __getitem__(self, name):
        return self.get().dereference()[name]

class FlatSegmentTreePrinter(object):
    
    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree'
        self.val = val

    def to_string(self):
        def key(name):
            return IntrusivePtr(self.val[name])['value_leaf']['key']
        return '%s [%d..%d]' % (self.typename, key('m_left_leaf'), key('m_right_leaf'))

    def children(self):
        return self.iterator(self.val['m_left_leaf'], self.val['m_right_leaf'])

    def display_hint(self):
        return 'map'

    class iterator(six.Iterator):

        def __init__(self, begin, end):
            self.node = IntrusivePtr(begin)
            self.end = IntrusivePtr(end)
            self.val = None
            self.have_val = False

        def __iter__(self):
            return self

        def __next__(self):
            if self.have_val:
                self.have_val = False
                return 'value', self.val
            if self.node == self.end:
                raise StopIteration
            start = self.node['value_leaf']['key']
            self.val, self.have_val = self.node['value_leaf']['value'], True
            self.node = IntrusivePtr(self.node['next'])
            return 'range', '%d..%d' % (start, self.node['value_leaf']['key'])


class FstIteratorPrinter(object):

    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree::iterator'
        self.val = val

    def to_string(self):
        pos = self.val['m_pos']
        if not pos:
            return 'singular %s' % self.typename
        node = pos.dereference()['value_leaf']
        return '%s [%s] = %s' % (self.typename, node['key'], node['value'])


class FstSegmentIteratorPrinter(object):

    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree::segment_iterator'
        self.val = val

    def to_string(self):
        if not self.val['m_start']:
            return 'singular %s' % self.typename
        node = self.val['m_node']
        return '%s [%s..%s] = %s' % (self.typename, node['start'],
                node['end'], node['value'])


def build_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter('mdds')

    pp.add_printer('flat_segment_tree', '^mdds::flat_segment_tree<.*>$', FlatSegmentTreePrinter)
    pp.add_printer('flat_segment_tree::iterator', '^mdds::flat_segment_tree<.*>::const_(reverse_)?iterator$', FstIteratorPrinter)
    pp.add_printer('flat_segment_tree::segment_iterator', '^mdds::__fst::const_segment_iterator<.*>$', FstSegmentIteratorPrinter)

    return pp

def register_pretty_printers(obj):
    gdb.printing.register_pretty_printer(obj, build_pretty_printers())

# vim:set shiftwidth=4 softtabstop=4 expandtab:
