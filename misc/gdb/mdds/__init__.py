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

import intervaltree

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


class MapIterator(six.Iterator):

    def __init__(self, iterable):
        self.iterable = iterable
        self.have_saved = False
        self.saved_val = None

    def __iter__(self):
        return self

    def __next__(self):
        if self.have_saved:
            self.have_saved = False
            return "", self.saved_val
        (key, self.saved_val) = six.next(self.iterable)
        self.have_saved = True
        return "", key


class FlatSegmentTreePrinter(object):
    
    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree'
        self.val = val

    def to_string(self):
        def key(name):
            return IntrusivePtr(self.val[name])['value_leaf']['key']
        return '%s [%d..%d]' % (self.typename, key('m_left_leaf'), key('m_right_leaf'))

    def children(self):
        return MapIterator(self.iterator(self.val['m_left_leaf'], self.val['m_right_leaf']))

    def display_hint(self):
        return 'map'

    class iterator(six.Iterator):

        def __init__(self, begin, end):
            self.node = IntrusivePtr(begin)
            self.end = IntrusivePtr(end)

        def __iter__(self):
            return self

        def __next__(self):
            if self.node == self.end:
                raise StopIteration
            start = self.node['value_leaf']['key']
            val = self.node['value_leaf']['value']
            self.node = IntrusivePtr(self.node['next'])
            return ('%d..%d' % (start, self.node['value_leaf']['key']), val)


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


class SortedStringMapPrinter(object):

    def __init__(self, val):
        self.typename = 'mdds::sorted_string_map'
        self.val = val

    def to_string(self):
        size = self.val['m_entry_size']
        if size == 0:
            return 'empty %s' % self.typename
        return '%s with %d values' % (self.typename, size)

    def children(self):
        return MapIterator(self.iterator(self.val['m_entries'], self.val['m_entry_size']))

    def display_hint(self):
        return 'map'

    class iterator(six.Iterator):

        def __init__(self, entry, count):
            self.val = entry
            self.count = count

        def __iter__(self):
            return self

        def __next__(self):
            if self.count <= 0:
                raise StopIteration
            val = self.val.dereference()
            self.val += 1
            self.count -= 1
            return (gdb.Value.lazy_string(val['key'], 'ascii', val['keylen']), val['value'])


class SegmentTreePrinter(object):

    def __init__(self, val):
        self.typename = 'mdds::segment_tree'
        self.val = val

    def to_string(self):
        try:
            it = self._segment_data()
            six.next(it)
        except StopIteration:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        return MapIterator(self.iterator(self._segment_data()))

    def _segment_data(self):
        sd_visualizer = gdb.default_visualizer(self.val['m_segment_data'])
        return iter(sd_visualizer.children() if sd_visualizer else [])

    class iterator(six.Iterator):

        def __init__(self, segment_data):
            tree = intervaltree.IntervalTree()
            for n, s in enumerate(segment_data):
                if n % 2 == 0:
                    val = s[1]
                else:
                    tree.addi(s[1]['first'], s[1]['second'], val)
            tree.split_overlaps()
            def append(a, i):
                a.append(i)
                return a
            tree.merge_equals(append, [])
            self.it = iter(sorted(tree.items(), key=lambda i: i.begin))

        def __iter__(self):
            return self

        def __next__(self):
            def format_list(data):
                return '[' + ', '.join(map(str, data)) + ']'
            val = six.next(self.it)
            return '%s..%s' % (val.begin, val.end), format_list(val.data)

    def display_hint(self):
        return 'map'

def build_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter('mdds')

    pp.add_printer('flat_segment_tree', '^mdds::flat_segment_tree<.*>$', FlatSegmentTreePrinter)
    pp.add_printer('flat_segment_tree::iterator', '^mdds::flat_segment_tree<.*>::const_(reverse_)?iterator$', FstIteratorPrinter)
    pp.add_printer('flat_segment_tree::segment_iterator', '^mdds::__fst::const_segment_iterator<.*>$', FstSegmentIteratorPrinter)
    pp.add_printer('segment_tree', '^mdds::segment_tree<.*>$', SegmentTreePrinter)
    pp.add_printer('sorted_string_map', '^mdds::sorted_string_map<.*>$', SortedStringMapPrinter)

    return pp

def register_pretty_printers(obj):
    gdb.printing.register_pretty_printer(obj, build_pretty_printers())

# vim:set shiftwidth=4 softtabstop=4 expandtab:
