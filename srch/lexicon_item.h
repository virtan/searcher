#ifndef LEXICON_ITEM_H
#define LEXICON_ITEM_H

#include <boost/array.hpp>
#include <srch/itim_base.h>

#pragma pack(push, 1)
namespace itim {

class lexicon_item {
    public:
    lexicon_item() : df(0), idf(0) {}
    lexicon_item(uint32_t _df, float _idf, boost::array<aindex_t, 6> _offsets) :
        df(_df), idf(_idf), offsets(_offsets) {}

    bool operator==(const lexicon_item &other) const {
        return df == other.df &&
               idf == other.idf &&
               offsets == other.offsets;
    }

    uint32_t df;
    float idf;
    boost::array<aindex_t, 6> offsets;
};

inline void print_lexicon_item_to_stream(std::ostream &s, const lexicon_item &l) {
    auto cond_out = [&] (size_t i) { l.offsets[i] == not_exists ?
        s << "not_exists" : s << l.offsets[i]; };
    s << "[";
    for(size_t i = 0; i < 6; ++i) {
        cond_out(i);
        if(i != 5) s << ',';
    }
    s << "],";
    s << l.df << ',' << l.idf;
}

inline std::ostream &operator<<(std::ostream &s, const lexicon_item &l) {
    s << '{';
    print_lexicon_item_to_stream(s, l);
    s << '}';
    return s;
}

class lexicon_item_and_term {
    public:
    lexicon_item_and_term(const term_type &_term, const lexicon_item &_item) :
        term(_term), item(_item) {}

    term_type term;
    lexicon_item item;
};

inline std::ostream &operator<<(std::ostream &s, const lexicon_item_and_term &l) {
    s << "{\"";
    s << l.term << "\",";
    print_lexicon_item_to_stream(s, l.item);
    s << '}';
    return s;
}

}
#pragma pack(pop)

#endif
