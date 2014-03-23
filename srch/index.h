#ifndef INDEX_H
#define INDEX_H

#include <boost/assign/list_of.hpp>
#include <srch/itim_base.h>
#include <srch/itim_algo.h>
#include <srch/itim_iterator_range.h>
#include <srch/config_branch.h>
#include <srch/lexicon.h>
#include <srch/posting_iterator.h>
#include <srch/mapped_region.h>
#include <srch/qatar_codecs.h>
#include <srch/grouping_codecs.h>
#include <srch/web_page_info_repos.h>
#include <srch/domain2site_repos.h>

namespace itim {

template<class index_data_accessor, pass_type ir, depth d>
class index_posting_iterator_range_generator;

template<class index_data_accessor, pass_type ir, block b>
class index_posting_iterator_range_block_generator;

template<class lexicon_data_accessor = mapped_region,
    class index_data_accessor = mapped_region>
class index {
    public:
    index(const config_branch &index_cfg) :
        id(index_cfg.get<size_t>("id")),
        lex(index_cfg.branch("lexicon")),
        ida(index_cfg.branch("index"), id),
        wpis(index_cfg.branch("webpageinfo")),
        d2s(index_cfg.branch("domain2site"))
    {}

    private:
    index(const index &);
    index(index &&);

    public:
    lexicon<lexicon_data_accessor> &lexicon_ref() { return lex; }

    // main call return values
    template<pass_type ir, depth d>
    struct range {
        typedef typename index_posting_iterator_range_generator<index_data_accessor, ir, d>::
            return_value value;
    };

    // main call
    // interface: range<iterator<...>> posting_iterator_range<pass_type, depth>(term)
    // example: posting_iterator_range<draft, first_block>(term)
    //          posting_iterator_range<full, all_blocks>(term)
    template<pass_type ir, depth d>
    typename range<ir, d>::value
    posting_iterator_range(const term_type &term) {
        index_posting_iterator_range_generator<index_data_accessor, ir, d> gen;
        return gen(term, *this);
    }

    template<pass_type ir, block b>
    typename index_posting_iterator_range_block_generator<index_data_accessor, ir, b>::
    return_value
    posting_iterator_range(const term_type &term) {
        index_posting_iterator_range_block_generator<index_data_accessor, ir, b> gen;
        return gen(term, *this);
    }

    // It's for internal use only
    template<class codec>
    posting_iterator<codec> specific_posting_iterator(const term_type &t, block b) {
        using boost::ref;
        using boost::cref;
        using boost::make_tuple;

        auto li = lex.find(itim::uppercase(t));
        log_debug_mare("lexicon[" << this->id << "].find(uc(\"" << t << "\")) = " <<
                (li == this->lex.end() ? "not found" : "found"));
        size_t wpis_corpus_size = wpis.get_corpus_size();
        assert(wpis_corpus_size > 0);
        return li == lex.end() ? posting_iterator<codec>() :
            posting_iterator<codec>(make_tuple(cref(li->second), cref(b), ref(ida), wpis_corpus_size));
    }

    /*
    uint64_t get_frequency(const string &term) {
        lexicon item = find(term);
        return item ? item.get_df() : 0;
    }
    
    size_t get_data_length() {
        return data.get_size();
    }

    float get_idf(const string &term) {
        lexicon item = find(term);
        return item ? item.get_idf() : 0;
    }

    private:
    lexicon_item find(const string &term) {
        return lex.find(term);
    }
    */


    public:
    size_t id;

    private:
    lexicon<lexicon_data_accessor> lex;
    index_data_accessor ida;

    public:
    web_page_info_repository wpis;
    domain2site_repository d2s;
};

template<class index_data_accessor, pass_type ir, depth d>
class index_posting_iterator_range_generator {};

namespace index_posting_iterator_range_generator_ns {

template<class return_value, class index_, class codec, class iterator>
return_value f_first_block(const term_type &t, index_ &i) {
    return itim::make_iterator_range(
            i.template specific_posting_iterator<codec>(t, b1k),
            iterator());
}

template<class return_value, class index_, class codec, class iterator,
    class internal_codec>
return_value f_top_blocks(const term_type &t, index_ &i) {
    return itim::make_iterator_range(
            iterator(
                (typename codec::initializers) boost::assign::list_of
                    (i.template specific_posting_iterator<internal_codec>(t, b1k))
                    (i.template specific_posting_iterator<internal_codec>(t, b100k))),
            iterator());
}

template<class return_value, class index_, class codec, class iterator,
    class internal_codec>
    return_value f_all_blocks(const term_type &t, index_ &i) {
        return itim::make_iterator_range(
                iterator(
                    (typename codec::initializers) boost::assign::list_of
                        (i.template specific_posting_iterator<internal_codec>(t, b1k))
                        (i.template specific_posting_iterator<internal_codec>(t, b100k))
                        (i.template specific_posting_iterator<internal_codec>(t, btail))),
                iterator());
    }
}

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, draft, first_block> {
    public:
    typedef qatar_draft_codec<index_data_accessor> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;

    template<class index_>
    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_first_block<return_value, index_, codec, iterator>(t, i);
    }
};

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, full, first_block> {
    public:
    typedef qatar_full_codec<index_data_accessor> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_first_block<return_value, index_, codec, iterator>(t, i);
    }
};

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, draft, top_blocks> {
    public:
    typedef qatar_draft_codec<index_data_accessor> internal_codec;
    typedef docid_merging_codec<posting_iterator<internal_codec>, 2> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_top_blocks<return_value, index_, codec, iterator, internal_codec>(t, i);
    }
};

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, full, top_blocks> {
    public:
    typedef qatar_full_codec<index_data_accessor> internal_codec;
    typedef docid_merging_codec<posting_iterator<internal_codec>, 2> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_top_blocks<return_value, index_, codec, iterator, internal_codec>(t, i);
    }
};

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, draft, all_blocks> {
    public:
    typedef qatar_draft_codec<index_data_accessor> internal_codec;
    typedef docid_merging_codec<posting_iterator<internal_codec>, 3> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_all_blocks<return_value, index_, codec, iterator, internal_codec>(t, i);
    }
};

template<class index_data_accessor>
class index_posting_iterator_range_generator<index_data_accessor, full, all_blocks> {
    public:
    typedef qatar_full_codec<index_data_accessor> internal_codec;
    typedef docid_merging_codec<posting_iterator<internal_codec>, 3> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            f_all_blocks<return_value, index_, codec, iterator, internal_codec>(t, i);
    }
};

template<class index_data_accessor, pass_type ir, block b>
class index_posting_iterator_range_block_generator {};

namespace index_posting_iterator_range_generator_ns {

template<class return_value, class index_, class codec, class iterator>
return_value specific_block(const term_type &t, index_ &i, block b) {
    return itim::make_iterator_range(
            i.template specific_posting_iterator<codec>(t, b),
            iterator());
}

}

template<class index_data_accessor, block b>
class index_posting_iterator_range_block_generator<index_data_accessor, draft, b> {
    public:
    typedef qatar_draft_codec<index_data_accessor> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;

    template<class index_>
    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            specific_block<return_value, index_, codec, iterator>(t, i, b);
    }
};

template<class index_data_accessor, block b>
class index_posting_iterator_range_block_generator<index_data_accessor, full, b> {
    public:
    typedef qatar_full_codec<index_data_accessor> codec;
    typedef posting_iterator<codec> iterator;
    typedef itim::iterator_range<iterator> return_value;
    typedef index<index_data_accessor> index_;

    return_value operator()(const term_type &t, index_ &i) {
        return index_posting_iterator_range_generator_ns::
            specific_block<return_value, index_, codec, iterator>(t, i, b);
    }
};

}

#endif
