#ifndef GROUPING_CODECS_H
#define GROUPING_CODECS_H

#include <boost/array.hpp>
#include <srch/itim_base.h>

namespace itim {

template<class fwd_iterator, size_t N>
class docid_merging_codec {
    public:
    typedef boost::array<fwd_iterator, N> it_array_t;
    typedef it_array_t initializers;
    typedef typename fwd_iterator::value_type posting_type;

    public:
    docid_merging_codec() : itar_all_at_end(true) {}
    docid_merging_codec(const initializers &in) : itar(in), current(0), itar_all_at_end(false) {
        find_minimal();
    }

    posting_type dereference() const {
        return *(itar[current]);
    }

    void increment() {
        ++itar[current];
        find_minimal();
    }

    bool equal(docid_merging_codec const &other) const {
        if(itar_all_at_end) return other.itar_all_at_end;
        if(other.itar_all_at_end) return itar_all_at_end;
        return current == current && itar == itar;
    }

    void lower_bound(docid_t min_docid) {
        if(itar_all_at_end) return;
        for(size_t i = 0; i < itar.size(); ++i)
            itar[i].lower_bound(min_docid);
        find_minimal();
    }

    size_t scanned() const {
        size_t d = 0;
        for(size_t i = 0; i < itar.size(); ++i)
            d += itar[i].scanned();
        return d;
    }
    
    size_t size() const {
        size_t d = 0;
        for(size_t i = 0; i < itar.size(); ++i)
            d += itar[i].size();
        return d;
    }

    private:
    void find_minimal() {
        if(itar_all_at_end) return;
        itar_all_at_end = true;
        for(size_t i = 0; i < itar.size(); ++i)
            if(itar[i] != internal_end_iterator &&
                    (itar_all_at_end || *(itar[i]) < *(itar[current]))) {
                current = i;
                itar_all_at_end = false;
            }
    }

    private:
    it_array_t itar;
    size_t current;
    fwd_iterator internal_end_iterator;
    bool itar_all_at_end;
};

}

#endif
