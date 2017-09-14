#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <sstream>

#include <mdds/flat_segment_tree.hpp>

#include <quickcheck/quickcheck.hh>

namespace
{

using quickcheck::Property;

using std::ostream;
using std::pair;
using std::string;

typedef mdds::flat_segment_tree<unsigned, unsigned> fst_type;
typedef fst_type::key_type key_type;
typedef fst_type::value_type value_type;

class fst_wrapper
{
public:
    fst_type* get() const
    {
        return m_pimpl.get();
    }
    fst_type& operator*() const
    {
        assert(initialized());
        return *get();
    }
    fst_type* operator->() const
    {
        assert(initialized());
        return get();
    }

    bool initialized() const
    {
        return m_pimpl.get();
    }

    void initialize(key_type low_, key_type high_, value_type value_)
    {
        assert(!initialized());
        m_pimpl.reset(new fst_type(low_, high_, value_));
    }

private:
    std::shared_ptr<fst_type> m_pimpl;
};

template<typename Key, typename Value>
ostream&
operator<<(ostream& out_, mdds::flat_segment_tree<Key, Value> const& fst_)
{
    out_ << "[";
    bool first(true);
    typedef typename mdds::flat_segment_tree<Key, Value>::const_iterator
        iterator_type;
    iterator_type cur(fst_.begin());
    for (iterator_type end(fst_.end()); cur != end; ++cur) 
    {
        if (first)
            first = false;
        else
            out_ << ", ";
        out_ << cur->first << ": " << cur->second;
    }
    out_ << ", " << cur->first << "), default = " << fst_.default_value();
    return out_;
}

ostream& operator<<(ostream& out_, fst_wrapper const& fst_)
{
    return fst_.initialized() ? out_ << *fst_ : out_;
}

template<typename T>
struct generator
{
    generator(size_t n_)
        : m_n(n_)
    {
    }

    T operator()() const
    {
        using quickcheck::generate;
        T val;
        generate(m_n, val);
        return val;
    }

private:
    size_t m_n;
};

void generate(size_t n_, fst_wrapper& fst_)
{
    using quickcheck::generate;
    value_type default_value;
    generate(n_, default_value);

    // generate a number of segments
    size_t segments(0);
    generate(n_, segments);
    segments = std::max<size_t>(segments, 1);

    using std::set;
    set<key_type> points;
    {
        size_t count(segments + 1);
        while (count > 0)
        {
            generator<key_type> gen(n_);
            if (points.insert(gen()).second)
                --count;
        }
    }

    using std::vector;
    vector<value_type> values;
    values.reserve(segments);
    std::generate_n(
            std::back_inserter(values), segments, generator<value_type>(n_));

    fst_.initialize(*points.begin(), *points.rbegin(), default_value);

    // insert segments
    if (segments >= 2)
    {
        assert(points.size() == values.size() + 1);

        typedef set<key_type>::const_iterator points_iterator;
        points_iterator last(points.begin());
        points_iterator cur(points.begin());
        points_iterator end(points.end());
        typedef vector<value_type>::const_iterator values_iterator;
        values_iterator cur_val(values.begin());
        values_iterator end_val(values.end());
        ++cur;
        while (cur != end && cur_val != end_val)
        {
            fst_->insert_back(*last, *cur, *cur_val);
            last = cur;
            ++cur;
            ++cur_val;
        }
    }
}

bool is_valid_key(fst_type const& fst_, key_type const& key_)
{
    return key_ >= fst_.begin()->first && key_ < fst_.rbegin()->first;
}

bool is_valid_range(fst_type const& fst_, key_type const& low_, key_type const& high_)
{
    return low_ < high_ && is_valid_key(fst_, low_) && is_valid_key(fst_, high_);
}

bool is_empty_tree(fst_type const& fst_)
{
    return fst_.rbegin()->first - fst_.begin()->first == 1;
}

bool is_single_segment(fst_type const& fst_)
{
    fst_type::const_iterator begin(fst_.begin());
    ++begin;
    return begin == fst_.end();
}

class FlatSegmentTreeInsertProperty
    : public Property<fst_wrapper, key_type, key_type, key_type>
{
public:
    typedef fst_type::const_iterator const_iterator;

    virtual bool
    accepts(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const&)
    {
        return is_valid_range(*fst_, low_, high_);
    }
};

class FlatSegmentTreeChangeProperty
    : public FlatSegmentTreeInsertProperty
{
public:
    virtual bool
    isTrivialFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const&)
    {
        return is_empty_tree(*fst_);
    }
};

class PInsertFrontIsSameAsInsertBack : public FlatSegmentTreeChangeProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type insert_front(*fst_);
        fst_type insert_back(*fst_);
        insert_front.insert_front(low_, high_, value_);
        insert_back.insert_back(low_, high_, value_);
        return insert_front == insert_back;
    }
};

class PInsertIsSameAsInsertBack : public FlatSegmentTreeChangeProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type insert(*fst_);
        fst_type insert_back(*fst_);
        insert.insert(insert.begin(), low_, high_, value_);
        insert_back.insert_back(low_, high_, value_);
        return insert == insert_back;
    }
};

class PRepeatedInsertDoesNothing : public FlatSegmentTreeChangeProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type fst(*fst_);
        const_iterator const pos(fst.insert_front(low_, high_, value_).first);
        fst_type orig(fst);
        fst.build_tree();
        pair<const_iterator, bool> result(fst.insert(pos, low_, high_, value_));
        return !result.second && result.first == pos && fst == orig;
    }
};

class PKeyInLimitsIsFound : public FlatSegmentTreeInsertProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type fst(*fst_);
        value_type dummy;
        pair<const_iterator, bool> result(fst.search(low_, dummy));

        return result.second && result.first != fst.end() &&
            result.first->first <= low_;
    }
};

class PKeyInInsertedSegmentIsFound : public FlatSegmentTreeChangeProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type fst(*fst_);
        pair<const_iterator, bool>
            insert(fst.insert_front(low_, high_, value_));
        value_type value;
        pair<const_iterator, bool> search(fst.search(insert.first, low_, value));
        return search.second && search.first == insert.first && value == value_;
    }
};

class PSearchTreeIsSameAsSearch : public Property<fst_wrapper, key_type>
{
    virtual bool holdsFor(fst_wrapper const& fst_, key_type const& key_)
    {
        fst_type fst(*fst_);
        value_type value;
        key_type low;
        key_type high;
        pair<fst_type::const_iterator, bool> search(fst.search(key_, value, &low, &high));

        if (!fst.is_tree_valid())
            fst.build_tree();
        value_type value_tree;
        key_type low_tree;
        key_type high_tree;
        bool const search_tree(fst.search_tree(key_, value_tree, &low_tree, &high_tree));
        return search.second == search_tree && low == low_tree
            && high == high_tree && value == value_tree;
    }

    virtual bool accepts(fst_wrapper const& fst_, key_type const& key_)
    {
        return is_valid_key(*fst_, key_);
    }
};

class PSearchDoesNotChangeTree : public Property<fst_wrapper, key_type>
{
    virtual bool holdsFor(fst_wrapper const& fst_, key_type const& key_)
    {
        fst_type& orig(*fst_);
        fst_type fst(orig);
        value_type dummy;
        fst.search(key_, dummy);
        return fst == orig;
    }

    virtual bool accepts(fst_wrapper const& fst_, key_type const& key_)
    {
        return is_valid_key(*fst_, key_);
    }
};

class PBuildTreeDoesNotChangeTree : public Property<fst_wrapper>
{
    virtual bool holdsFor(fst_wrapper const& fst_)
    {
        fst_type& orig(*fst_);
        fst_type& fst(orig);
        fst.build_tree();
        return fst == orig;
    }

    virtual bool isTrivialFor(fst_wrapper const& fst_)
    {
        return fst_->is_tree_valid();
    }
};

class PShiftLeftRemovesSomething : public Property<fst_wrapper, key_type, key_type>
{
    virtual bool
    holdsFor(fst_wrapper const& fst_, key_type const& low_, key_type const& high_)
    {
        fst_type& orig(*fst_);
        fst_type fst(orig);
        fst.shift_left(low_, high_);
        return fst != orig;
    }

    virtual bool
    accepts(fst_wrapper const& fst_, key_type const& low_, key_type const& high_)
    {
        return is_valid_range(*fst_, low_, high_) && !is_single_segment(*fst_);
    }
};

class PShiftRightWorks : public Property<fst_wrapper, key_type, key_type, bool>
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& key_, key_type const& size_,
            bool const& skip_start_)
    {
        fst_type& orig(*fst_);
        fst_type fst(orig);
        fst.shift_right(key_, size_, skip_start_);
        return isTrivialFor(fst_, key_, size_, skip_start_) ? fst == orig : fst != orig;
    }

    virtual bool
    accepts(
            fst_wrapper const& fst_, key_type const& key_, key_type const& size_,
            bool const&)
    {
        return is_valid_key(*fst_, key_);
    }

    virtual bool
    isTrivialFor(
            fst_wrapper const& fst_, key_type const& key_, key_type const& size_,
            bool const& skip_start_)
    {
        return
            (is_single_segment(*fst_) && fst_->begin()->second == fst_->default_value())
            || size_ == 0
            || (skip_start_ && key_ == fst_->max_key() - 1);
    }
};

bool keys_ordered(fst_type const& fst_)
{
    fst_type::const_iterator cur(fst_.begin());
    fst_type::const_iterator end(fst_.end());
    fst_type::const_iterator last(cur);
    ++cur;
    while (cur != end)
    {
        if (last->first >= cur->first)
            return false;
        ++cur;
    }
    return last->first < cur->first;
}

class PKeysOrderedAfterInsert : public FlatSegmentTreeInsertProperty
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type fst(*fst_);
        fst.insert_front(low_, high_, value_);
        return keys_ordered(fst);
    }

    virtual bool
    isTrivialFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            value_type const& value_)
    {
        fst_type fst(*fst_);
        return !fst.insert_front(low_, high_, value_).second;
    }
};

class PKeysOrderedAfterShiftLeft : public Property<fst_wrapper, key_type, key_type>
{
    virtual bool
    holdsFor(fst_wrapper const& fst_, key_type const& low_, key_type const& high_)
    {
        fst_type fst(*fst_);
        fst.shift_left(low_, high_);
        return keys_ordered(fst);
    }

    virtual bool
    accepts(fst_wrapper const& fst_, key_type const& low_, key_type const& high_)
    {
        return is_valid_range(*fst_, low_, high_);
    }

    virtual bool
    isTrivialFor(fst_wrapper const& fst_, key_type const& low_, key_type const& high_)
    {
        fst_type& fst(*fst_);
        return is_single_segment(fst) && fst.begin()->second == fst.default_value();
    }
};

class PKeysOrderedAfterShiftRight : public Property<fst_wrapper, key_type, key_type, bool>
{
    virtual bool
    holdsFor(
            fst_wrapper const& fst_, key_type const& key_, key_type const& size_,
            bool const& skip_start_)
    {
        fst_type fst(*fst_);
        fst.shift_right(key_, size_, skip_start_);
        return keys_ordered(fst);
    }

    virtual bool
    accepts(
            fst_wrapper const& fst_, key_type const& key_, key_type const& size_,
            bool const&)
    {
        return is_valid_key(*fst_, key_);
    }

    virtual bool
    isTrivialFor(
            fst_wrapper const& fst_, key_type const& low_, key_type const& high_,
            bool const&)
    {
        fst_type& fst(*fst_);
        return is_single_segment(fst) && fst.begin()->second == fst.default_value();
    }
};

}

int main()
{
    size_t const tests(200);
    using quickcheck::check;
    check<PInsertFrontIsSameAsInsertBack>(
            "insert_front is the same as insert_back", tests);
    check<PInsertIsSameAsInsertBack>("insert is same as insert_back", tests);
    check<PRepeatedInsertDoesNothing>("repeated insert does nothing", tests);
    check<PKeyInLimitsIsFound>("key in tree limits is found", tests);
    check<PKeyInInsertedSegmentIsFound>(
            "key in just inserted segment is found at the right pos", tests);
    check<PSearchTreeIsSameAsSearch>("search_tree is the same as search", tests);
    check<PSearchDoesNotChangeTree>("search does not change tree", tests);
    check<PBuildTreeDoesNotChangeTree>("build_tree does not change tree", tests);
    check<PShiftLeftRemovesSomething>("shift_left removes something", tests);
    // check<PShiftRightWorks>("shift_right works", tests);
    check<PKeysOrderedAfterInsert>("keys remain ordered after insert", tests);
    check<PKeysOrderedAfterShiftLeft>("keys remain ordered after shift_left", tests);
    check<PKeysOrderedAfterShiftRight>("keys remain ordered after shift_right", tests);
}

// vim: set ts=4 sw=4 et:
