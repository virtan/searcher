#ifndef POSTING_ITERATOR_H
#define POSTING_ITERATOR_H

#include <boost/iterator/iterator_facade.hpp>
#include <srch/index_data_iterator.h>

namespace itim {

template<class codec>
class posting_iterator :
    public boost::iterator_facade<
        posting_iterator<codec>,
        const typename codec::posting_type &,
        boost::forward_traversal_tag>
{
    public:
    posting_iterator() {}
    posting_iterator(typename codec::initializers in) : cdc(in) {}

    private:
    friend class boost::iterator_core_access;

    const typename codec::posting_type &dereference() const { return cdc.dereference(); }
    void increment() { cdc.increment(); }
    bool equal(posting_iterator const &other) const { return cdc.equal(other.cdc); }

    public:
    void lower_bound(const docid_t &min_docid) { cdc.lower_bound(min_docid); }
    size_t scanned() const { return cdc.scanned(); }
    size_t size() const { return cdc.size(); }

    private:
    codec cdc;
};

}

#endif
