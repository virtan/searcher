#ifndef POSTING_LIST_H
#define POSTING_LIST_H

#include <srch/posting_iterator.h>
#include <srch/qatar_draft_codec.h>
#include <srch/qatar_full_codec.h>

namespace itim {

template<class index_data_accessor>
class posting_list {
    public:
    // enum depth { top, middle, total };

    posting_list() : empty_(true) {}
    posting_list(const lexicon_item &_lex_item, index_data_accessor &_ida) :
        lex_item(_lex_item), ida(_ida), empty_(false) {}

    operator bool() const { return empty_; }

    template<codec>
    posting_iterator<draft_codec> postings(block b) {
        assert(!empty_);
        // index_data_iterator begin(ida, lex_item.bucket[bucket].draft_offset);
        //index_data_iterator end(ida, lex_item.bucket[bucket].draft_offset +
        //        lex_item.bucket[bucket].draft_length);
        return posting_iterator<draft_codec>(b, lex_item, ida);
    }

    template<full_codec = qatar_full_codec>
    posting_iterator<full_codec> full_postings(block b) {
        assert(!empty_);
        //index_data_iterator begin(ida, lex_item.bucket[bucket].full_offset);
        //index_data_iterator end(ida, lex_item.bucket[bucket].full_offset +
        //        lex_item.bucket[bucket].full_length);
        return posting_iterator<full_codec>(b, lex_item, ida);
    }

    private:
    lexicon_item lex_item;
    index_data_accessor &ida;
    bool empty_;
};

}

#endif
