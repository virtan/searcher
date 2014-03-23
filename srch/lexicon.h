#ifndef LEXICON_H
#define LEXICON_H

#include <eger/profiler.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/lexicon_item.h>
#include <srch/qatar_codecs.h>

namespace itim {

#pragma pack(push, 1)
template<class index_data_accessor = mapped_region, class lexicon_codec = qatar_lexicon_codec<index_data_accessor> >
class lexicon : public std::unordered_map<term_type, lexicon_item> {
    public:
    lexicon(const config_branch &_cfg) :
        cfg(_cfg)
    {
        build();
    }

    void build() {
        index_data_accessor ida(cfg);
        auto iter_range = itim::make_iterator_range(
                index_data_iterator<index_data_accessor>(ida),
                index_data_iterator<index_data_accessor>(ida, ida.size()));
        lexicon_codec codec(iter_range);
#ifdef unordered_map_reserve
        this->reserve(ida.size()/40); // approximation
#endif
        profiler_start(lexicon_loading);
        try {
            while(true) {
                lexicon_item_and_term item_term = codec.next();
                (*this)[item_term.term] = item_term.item;
            }
        } catch(out_of_range_exception &e) {
            // end of lexicon_index
        }
        profiler_dump_2(lexicon_loading, this->size() << " items");
    }

    private:
    config_branch cfg;
};
#pragma pack(pop)

}

#endif
