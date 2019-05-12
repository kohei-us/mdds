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

import itertools

import six


def from_shared_ptr(val):
    try:
        delegate = gdb.default_visualizer(val)
        return delegate.pointer
    except:
        ptr = val['_M_ptr']
        if ptr:
            return ptr
    return val


class IntrusivePtr(object):
    """boost::intrusive_ptr wrapper."""

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


def map_iterator(iterable):
    """Convert an iterator over (key, value) pairs to gdb 'map' iterator.

    The gdb iterator protocol for pretty printer with display type
    'map'--for some reason unknown to me--expects the key and the value
    separately, each with its own 'key'. To avoid complicating all map
    iterators, this simple adapter is provided.
    """
    for (k, v) in iterable:
        yield "", k
        yield "", v


class inverse_map_iterator(six.Iterator):
    """The inverse of map_iterator."""

    def __init__(self, iterable):
        self.iterable = iterable

    def __iter__(self):
        return self

    def __next__(self):
        (_, key) = six.next(self.iterable)
        (_, val) = six.next(self.iterable)
        return key, val


def array_iterator(iterable):
    """Convert an iterator to gdb 'array' iterator."""
    for n, v in enumerate(iterable):
        yield str(n), v


def inverse_array_iterator(iterable):
    """Pick the values from a gdb 'array' iterator."""
    for _, v in iterable:
        yield v


class FlatSegmentTreePrinter(object):
    """Pretty printer for flat_segment_tree."""
    
    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree'
        self.val = val

    def to_string(self):
        def key(name):
            return IntrusivePtr(self.val[name])['value_leaf']['key']
        return '%s [%d..%d]' % (self.typename, key('m_left_leaf'), key('m_right_leaf'))

    def children(self):
        return map_iterator(self.iterator(self.val['m_left_leaf'], self.val['m_right_leaf']))

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


class FlatSegmentTreeIteratorPrinter(object):
    """Pretty printer for flat_segment_tree iterator."""

    def __init__(self, val):
        self.typename = 'mdds::flat_segment_tree::iterator'
        self.val = val

    def to_string(self):
        pos = self.val['m_pos']
        if not pos:
            return 'singular %s' % self.typename
        node = pos.dereference()['value_leaf']
        return '%s [%s] = %s' % (self.typename, node['key'], node['value'])


class FlatSegmentTreeSegmentIteratorPrinter(object):
    """Pretty printer for flat_segment_tree segment_iterator."""

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
    """Pretty printer for sorted_string_map."""

    def __init__(self, val):
        self.typename = 'mdds::sorted_string_map'
        self.val = val

    def to_string(self):
        size = self.val['m_entry_size']
        if size == 0:
            return 'empty %s' % self.typename
        return '%s with %d values' % (self.typename, size)

    def children(self):
        return map_iterator(self.iterator(self.val['m_entries'], self.val['m_entry_size']))

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
    """Pretty printer for segment_tree."""

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
        return map_iterator(self.iterator(self._segment_data()))

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


class SegmentTreeSearchResultPrinter(object):
    """Pretty printer for segment_tree search_result."""

    def __init__(self, val):
        self.typename = 'mdds::segment_tree::search_result'
        self.val = from_shared_ptr(val['mp_res_chains'])

    def to_string(self):
        if self.val.type.code == gdb.TYPE_CODE_PTR and not self.val:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        if self.val.type.code != gdb.TYPE_CODE_PTR or not self.val:
            return []
        def ptr_children(ptr):
            return gdb.default_visualizer(ptr.dereference()).children()
        data_chains = map(ptr_children, map(lambda t: t[1], ptr_children(self.val)))
        values = map(lambda t: t[1], itertools.chain.from_iterable(data_chains))
        return array_iterator(values)

    def display_hint(self):
        if self.val.type.code == gdb.TYPE_CODE_PTR:
            return 'array'
        return ''


class TrieNodeIterator(six.Iterator):
    """Iterator over trie_map nodes."""

    def __init__(self, node, prefix):
        nodes = gdb.default_visualizer(node['children']).children()
        children = (self.__class__(v, prefix + chr(k)) for k, v in inverse_map_iterator(nodes))
        self.children = itertools.chain.from_iterable(children)
        if node['has_value']:
            this = iter([('"%s"' % prefix, node['value'])])
            self.children = itertools.chain(this, self.children)

    def __iter__(self):
        return self

    def __next__(self):
        return six.next(self.children)


class TrieMapPrinter(object):
    """Pretty printer for trie_map."""

    def __init__(self, val):
        self.typename = 'mdds::trie_map'
        self.val = val

    def to_string(self):
        try:
            root_vis = gdb.default_visualizer(self.val['m_root']['children'])
            six.next(root_vis.children())
        except StopIteration:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        return map_iterator(TrieNodeIterator(self.val['m_root'], ""))

    def display_hint(self):
        return 'map'


class TrieMapSearchResultsPrinter(object):
    """Pretty printer for trie_map search_results."""

    def __init__(self, val):
        self.typename = 'mdds::trie_map::search_results'
        self.val = val

    def to_string(self):
        if self.val['m_node'] == 0:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        node = self.val['m_node']
        if node == 0:
            return []
        prefix = str(self.val['m_buffer']).strip('"')
        return map_iterator(TrieNodeIterator(node.dereference(), prefix))

    def display_hint(self):
        return 'map'


class TrieMapIteratorPrinter(object):
    """Pretty printer for trie_map iterator."""

    def __init__(self, val):
        trie_type = val.type.template_argument(0).name.partition('<')[0]
        self.typename = '%s::iterator' % trie_type
        self.val = val

    def to_string(self):
        typ = self.val['m_type']
        if typ == 0: # normal
            return '%s = %s' % (self.typename, self.val['m_current_value'])
        elif typ == 1: # end
            return 'non-dereferenceable %s' % self.typename
        elif typ == 2: # empty
            return 'singular %s' % self.typename


class PackedTrieMapIterator(six.Iterator):
    """Iterator over a packed_trie_map node."""

    def __init__(self, packed, ptr_type, node_pos, key=None):
        if key is None:
            key = ""
        index_size = packed[node_pos + 1]
        children = self.__iter_node_children(packed, ptr_type, key, node_pos, index_size)
        self.children = itertools.chain.from_iterable(children)
        if packed[node_pos] != 0:
            val_ptr = packed[node_pos].cast(ptr_type)
            this = iter([('"%s"' % key, val_ptr.dereference())])
            self.children = itertools.chain(this, self.children)

    def __iter_node_children(self, packed, ptr_type, key, node_pos, index_size):
        off = node_pos + 2
        node_end = off + index_size
        while off < node_end:
            c = packed[off]
            child_offset = packed[off + 1]
            off += 2
            yield self.__class__(packed, ptr_type, node_pos - child_offset, key + chr(c))

    def __iter__(self):
        return self

    def __next__(self):
        return six.next(self.children)


class PackedTrieMapPrinter(object):
    """Pretty printer for packed_trie_map."""

    def __init__(self, val):
        self.typename = 'mdds::packed_trie_map'
        self.val = val

    def to_string(self):
        if self.val['m_entry_size'] == 0:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        packed = [v for _, v in gdb.default_visualizer(self.val['m_packed']).children()]
        root_offset = packed[0]
        ptr_type = self.val['m_value_store'].type.template_argument(0).pointer()
        return map_iterator(PackedTrieMapIterator(packed, ptr_type, root_offset))

    def display_hint(self):
        return 'map'


class PackedTrieMapSearchResultsPrinter(object):
    """Pretty printer for packed_trie_map search_results."""

    def __init__(self, val):
        self.typename = 'mdds::packed_trie_map::search_results'
        self.val = val

    def to_string(self):
        if self.val['m_node'] == 0:
            return 'empty %s' % self.typename
        return self.typename

    class ptr_as_array(object):
        """Adapter allowing to treat a gdb.Value pointer as an array."""

        def __init__(self, ptr):
            self.ptr = ptr

        def __getitem__(self, n):
            p = self.ptr
            p += n
            return p.dereference()

    def children(self):
        if self.val['m_node'] == 0:
            return []
        array = self.ptr_as_array(self.val['m_node'])
        ptr_type = self.val.type.template_argument(0).template_argument(1).pointer()
        prefix = str(self.val['m_buffer']).strip('"')
        return map_iterator(PackedTrieMapIterator(array, ptr_type, 0, prefix))

    def display_hint(self):
        return 'map'


mtv_type_map = {}


class MultiTypeVectorBlockIterator(six.Iterator):
    """Iterator for multi_type_vector blocks."""

    def __init__(self, block, size, type_map):
        if block == 0:
            type_id = -1
        else:
            type_id = int(block['type'])
        elt_type = type_map[type_id]() if type_id in type_map else None
        if type_id == -1:
            elts = itertools.repeat('empty', size)
        elif elt_type is None:
            elts = ['***ERROR: block of unknown type id %d***' % type_id]
        else:
            data = block.cast(elt_type.pointer())
            array = data.dereference()['m_array']
            elts = inverse_array_iterator(gdb.default_visualizer(array).children())
        self.elts = iter(elts)

    def __iter__(self):
        return self

    def __next__(self):
        return six.next(self.elts)


class MultiTypeVectorPrinter(object):
    """Pretty printer for multi_type_vector."""

    def __init__(self, val, type_map):
        self.typename = 'mdds::multi_type_vector'
        self.val = val
        self.type_map = type_map

    @classmethod
    def with_type_map(cls, type_map):
        def build(val):
            return cls(val, type_map)
        return build

    def to_string(self):
        size = self.val['m_cur_size']
        if size == 0:
            return 'empty %s' % self.typename
        return '%s with %d elements' % (self.typename, size)

    def children(self):
        blocks_vis = gdb.default_visualizer(self.val['m_blocks'])
        blocks = (MultiTypeVectorBlockIterator(block['mp_data'], block['m_size'], self.type_map) for (_, block) in blocks_vis.children())
        return array_iterator(itertools.chain.from_iterable(blocks))

    def display_hint(self):
        return 'array'

    class make_type(object):
        """Helper for creating element_block types."""

        def __init__(self, block_name, type_id, names):
            self.block_name = block_name
            self.type_id = type_id
            if isinstance(names, tuple):
                self.names = names
            else:
                self.names = [names]

        def __call__(self):
            for name in self.names:
                try:
                    return self.__lookup_type(name)
                except:
                    try:
                        return self.__lookup_type(gdb.lookup_type(name).strip_typedefs())
                    except:
                        pass
            return None

        def __lookup_type(self, name):
            return gdb.lookup_type('%s<%d, %s>' % (self.block_name, self.type_id, name))

    @staticmethod
    def make_default_type_map():
        types = (
            'bool',
            'int8_t', 'uint8_t', 'int16_t', 'uint16_t', 'int32_t', 'uint32_t', 'int64_t', 'uint64_t',
            'float', 'double',
            ('std::string', 'std::__cxx11::string',)
        )
        make_type = MultiTypeVectorPrinter.make_type
        return {i: make_type('mdds::mtv::default_element_block', i, t) for i, t in enumerate(types)}


class MultiTypeVectorIteratorPrinter(object):
    """Pretty printer for multi_type_vector iterator."""

    def __init__(self, val, type_map):
        self.typename = 'mdds::multi_type_vector::iterator'
        self.val = val
        self.type_map = type_map

    @classmethod
    def with_type_map(cls, type_map):
        def build(val):
            return cls(val, type_map)
        return build

    def to_string(self):
        if self.val['m_cur_node']['data'] == 0 and self.val['m_cur_node']['size'] == 0:
            return 'non-dereferenceable %s' % self.typename
        return self.typename

    def children(self):
        data = self.val['m_cur_node']['data']
        size = self.val['m_cur_node']['size']
        if data == 0 and size == 0:
            return []
        return array_iterator(MultiTypeVectorBlockIterator(data, size, self.type_map))

    def display_hint(self):
        return 'array'


class PointQuadTreePrinter(object):
    """Pretty printer for point_quad_tree."""

    def __init__(self, val):
        self.typename = 'mdds::point_quad_tree'
        self.val = val

    def to_string(self):
        if IntrusivePtr(self.val['m_root']).get() == 0:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        def iter_level(root):
            if root.get() == 0:
                return iter([])
            r = root.get().dereference()
            dirs = [r['northeast'], r['northwest'], r['southeast'], r['southwest']]
            lower = map(iter_level, map(IntrusivePtr, dirs))
            this = ('%s, %s' % (r['x'], r['y']), r['data'])
            return itertools.chain([this], itertools.chain.from_iterable(lower))
        return map_iterator(iter_level(IntrusivePtr(self.val['m_root'])))

    def display_hint(self):
        return 'map'


class PointQuadTreeSearchResultsPrinter(object):
    """Pretty printer for point_quad_tree search_results."""

    def __init__(self, val):
        self.typename = 'mdds::point_quad_tree::search_results'
        self.val = val

    def to_string(self):
        if from_shared_ptr(self.val['mp_res_nodes']) == 0:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        def iter_nodes(nodes):
            for node in inverse_array_iterator(gdb.default_visualizer(nodes).children()):
                yield '[%s, %s] = %s' % (node['x'], node['y'], node['data'])
        nodes = from_shared_ptr(self.val['mp_res_nodes'])
        if nodes == 0:
            return []
        return array_iterator(iter_nodes(nodes.dereference()))

    def display_hint(self):
        return 'array'


class MultiTypeMatrixPrinter(object):
    """Pretty printer for multi_type_matrix."""

    def __init__(self, val):
        self.typename = 'mdds::multi_type_matrix'
        self.val = val

    def to_string(self):
        cols, rows = self.val['m_size']['column'], self.val['m_size']['row']
        if cols == 0 or rows == 0:
            return 'empty %s' % self.typename
        return '%s (%d columns, %d rows)' % (self.typename, cols, rows)

    def children(self):
        return gdb.default_visualizer(self.val['m_store']).children()

    def display_hint(self):
        return 'array'


class RTreePrinter(object):
    """Pretty printer for rtree."""

    def __init__(self, val):
        self.typename = 'mdds::rtree'
        self.val = val

    def to_string(self):
        if self.val['m_root']['count'] == 0:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        def iter_node_store(store, dir_type, val_type):
            if store['type'] in (1, 2): # directory
                if store['count'] == 0:
                    return
                node = store['node_ptr'].cast(dir_type.pointer()).dereference()
                for s in inverse_array_iterator(gdb.default_visualizer(node['children']).children()):
                    for val in iter_node_store(s, dir_type, val_type):
                        yield val
            elif store['type'] == 3: # value
                yield self.value(store, val_type)
            else: # unspecified or unknown type
                return

        dir_type = gdb.lookup_type('%s::directory_node' % self.val.type.strip_typedefs())
        val_type = gdb.lookup_type('%s::value_node' % self.val.type.strip_typedefs())
        return map_iterator(iter_node_store(self.val['m_root'], dir_type, val_type))

    def display_hint(self):
        return 'map'

    @staticmethod
    def value(store, value_type):
        val = store['node_ptr'].cast(value_type.pointer()).dereference()['value']
        def fmt_point(point):
            return store['extent'][point]['d']
        key = '%s - %s' % (fmt_point('start'), fmt_point('end'))
        return key, val


class RTreeSearchResultsPrinter(object):
    """Pretty printer for rtree search_results."""

    def __init__(self, val):
        self.typename = 'mdds::rtree::search_results'
        self.val = val

    def to_string(self):
        try:
            six.next(gdb.default_visualizer(self.val['m_store']).children())
        except:
            return 'empty %s' % self.typename
        return self.typename

    def children(self):
        def iter_values(store):
            base_type = str(self.val.type.strip_typedefs()).rstrip('::search_results')
            val_type = gdb.lookup_type(base_type + '::value_node')
            for v in inverse_array_iterator(gdb.default_visualizer(store).children()):
                store = v['ns'].dereference()
                yield RTreePrinter.value(store, val_type)
        return map_iterator(iter_values(self.val['m_store']))

    def display_hint(self):
        return 'map'


class RTreeIteratorPrinter(object):
    """Pretty printer for rtree iterator."""

    def __init__(self, val):
        self.typename = 'mdds::rtree::iterator'
        self.val = val

    def to_string(self):
        # NOTE: This is just wishful thinking. In reality there's no way to recognize a
        # non-dereferenceable iterator unless the standard library is used in debug mode.
        if self.__from_vector_iterator(self.val['m_pos']) is None:
            return 'non-dereferenceable %s' % self.typename
        return self.typename

    def children(self):
        base_type = str(self.val.type.strip_typedefs()).rstrip('::iterator')
        val_type = gdb.lookup_type(base_type + '::value_node')
        content = self.__from_vector_iterator(self.val['m_pos'])
        if content is None:
            return []
        store = content['ns'].dereference()
        return map_iterator([RTreePrinter.value(store, val_type)])

    def display_hint(self):
        return 'map'

    def __from_vector_iterator(self, iter):
        # HACK: this relies on details of standard library impl.
        fields = list(iter.type.fields())
        if len(fields) == 1:
            val = iter[fields[0].name]
            if val:
                return val.dereference()
        return None


def build_pretty_printers():
    pp = gdb.printing.RegexpCollectionPrettyPrinter('mdds')

    pp.add_printer('flat_segment_tree', '^mdds::flat_segment_tree<.*>$', FlatSegmentTreePrinter)
    pp.add_printer('flat_segment_tree::iterator',
            '^mdds::flat_segment_tree<.*>::const_(reverse_)?iterator$',
            FlatSegmentTreeIteratorPrinter)
    pp.add_printer('flat_segment_tree::segment_iterator',
            '^mdds::__fst::const_segment_iterator<.*>$',
            FlatSegmentTreeSegmentIteratorPrinter)

    pp.add_printer('multi_type_matrix', '^mdds::multi_type_matrix<.*>$', MultiTypeMatrixPrinter)

    pp.add_printer('multi_type_vector',
            '^mdds::multi_type_vector<.*>$',
            MultiTypeVectorPrinter.with_type_map(mtv_type_map))
    pp.add_printer('multi_type_vector::iterator',
            '^mdds::detail::mtv::(const_)?iterator_base<mdds::multi_type_vector<.*>$',
            MultiTypeVectorIteratorPrinter.with_type_map(mtv_type_map))

    pp.add_printer('packed_trie_map', '^mdds::packed_trie_map<.*>$', PackedTrieMapPrinter)
    pp.add_printer('packed_trie_map::iterator',
            '^mdds::trie::detail::packed_iterator_base<mdds::packed_trie_map<.*>$',
            TrieMapIteratorPrinter)
    pp.add_printer('packed_trie_map::search_results',
            '^mdds::trie::detail::packed_search_results<mdds::packed_trie_map<.*>$',
            PackedTrieMapSearchResultsPrinter)

    pp.add_printer('point_quad_tree', '^mdds::point_quad_tree<.*>$', PointQuadTreePrinter)
    pp.add_printer('point_quad_tree::search_results',
            '^mdds::point_quad_tree<.*>::search_results$',
            PointQuadTreeSearchResultsPrinter)

    pp.add_printer('rtree', '^mdds::rtree<.*>$', RTreePrinter)
    pp.add_printer('rtree::iterator', '^mdds::rtree<.*>::(const_)?iterator$', RTreeIteratorPrinter)
    pp.add_printer('rtree::search_results',
            '^mdds::rtree<.*>::(const_)?search_results$',
            RTreeSearchResultsPrinter)

    pp.add_printer('segment_tree', '^mdds::segment_tree<.*>$', SegmentTreePrinter)
    pp.add_printer('segment_tree::search_result',
            '^mdds::segment_tree<.*>::search_result$',
            SegmentTreeSearchResultPrinter)

    pp.add_printer('sorted_string_map', '^mdds::sorted_string_map<.*>$', SortedStringMapPrinter)

    pp.add_printer('trie_map', '^mdds::trie_map<.*>$', TrieMapPrinter)
    pp.add_printer('trie_map::iterator',
            '^mdds::trie::detail::iterator_base<mdds::trie_map<.*>$',
            TrieMapIteratorPrinter)
    pp.add_printer('trie_map::search_results',
            '^mdds::trie::detail::search_results<mdds::trie_map<.*>$',
            TrieMapSearchResultsPrinter)

    return pp


def register_pretty_printers(obj):
    gdb.printing.register_pretty_printer(obj, build_pretty_printers())


def register_mtv_types(type_map):
    """Register given type IDs for use with multi_type_vector.

    This is necessary to do for any non-default type ID, otherwise
    MultiTypeVectorPrinter won't be able to print values of the
    corresponding type.
    """

    mtv_type_map.update(type_map)


register_mtv_types(MultiTypeVectorPrinter.make_default_type_map())

# vim:set shiftwidth=4 softtabstop=4 expandtab:
