#ifndef OTHER_PHRASE_ITERATOR_H
#define OTHER_PHRASE_ITERATOR_H

#include <srch/itim_base.h>
#include <srch/phrase_iterator.h>
#include <srch/synonym_iterator.h>
#include <srch/itim_iterator_range.h>

namespace itim {

    // main_synonym_iterator
    define_phrase_iterator_header(
            main_synonym_iterator,
            define_pi_subiterator(synonym_isec_iterator),
            define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(main_synonym_iterator);
        use_pi_empty_constructor_m(main_synonym_iterator);
        use_pi_constructor_m(main_synonym_iterator);
        use_pi_lower_bound_merge_m;
        use_pi_make_range_m;

        protected:
        virtual void fill_ranges(index_c &ind) {
            //this->ranges.reserve(this->source->main_synonym_offs.size());
            for(query_group::main_synonym_offs_t::const_iterator i = this->source->main_synonym_offs.begin();
                    i != this->source->main_synonym_offs.end(); ++i)
                this->ranges.emplace_back(this->source_item_to_range(ind, (*(this->source))[*i]));
        }

    };


    // main_group_iterator
    define_phrase_iterator_header(main_group_iterator,
                           define_pi_subiterator(synonym_merge_iterator),
                           define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(main_group_iterator);
        use_pi_empty_constructor_m(main_group_iterator);
        use_pi_constructor_m(main_group_iterator);
        use_pi_lower_bound_merge_sylls_m;
        use_pi_fill_score_table_using_synonym_infos_m;
        use_pi_make_range_m;
        use_pi_lower_bound_merge_sylls_accurate_min_m;
        use_pi_lower_bound_merge_sylls_complete_check_m;
        use_pi_msynonym_extra_constructor_m;

    };

    define_phrase_iterator_extra_data_header(main_group_iterator)
    {
        typedef typename main_synonym_iterator<pt, d, index_c>::type msynonym_iterator_t;
        typedef itim::iterator_range<msynonym_iterator_t> msynonym_range_t;
        msynonym_range_t msynonym_range;
    };

    // fa_main_group_iterator
    define_phrase_iterator_header(fa_main_group_iterator,
                           define_pi_subiterator(synonym_isec_iterator),
                           define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(fa_main_group_iterator);
        use_pi_empty_constructor_m(fa_main_group_iterator);
        use_pi_constructor_m(fa_main_group_iterator);
        use_pi_lower_bound_merge_sylls_m;
        use_pi_fill_score_table_using_synonym_infos_m;
        use_pi_make_range_m;
        use_pi_lower_bound_merge_sylls_accurate_min_m;
        use_pi_lower_bound_merge_sylls_complete_check_m;
        use_pi_msynonym_extra_constructor_m;

    };

    define_phrase_iterator_extra_data_header(fa_main_group_iterator)
    {
        typedef typename main_synonym_iterator<pt, d, index_c>::type msynonym_iterator_t;
        typedef itim::iterator_range<msynonym_iterator_t> msynonym_range_t;
        msynonym_range_t msynonym_range;
    };


    // other_group_iterator
    define_phrase_iterator_header(other_group_iterator,
                           define_pi_subiterator(synonym_merge_iterator),
                           define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(other_group_iterator);
        use_pi_empty_constructor_m(other_group_iterator);
        use_pi_constructor_m(other_group_iterator);
        use_pi_lower_bound_merge_sylls_m;
        use_pi_fill_score_table_using_synonym_infos_m;
        use_pi_make_range_m;
    };

    // fa_other_group_iterator
    define_phrase_iterator_header(fa_other_group_iterator,
                           define_pi_subiterator(synonym_isec_iterator),
                           define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(fa_other_group_iterator);
        use_pi_empty_constructor_m(fa_other_group_iterator);
        use_pi_constructor_m(fa_other_group_iterator);
        use_pi_lower_bound_merge_sylls_m;
        use_pi_fill_score_table_using_synonym_infos_m;
        use_pi_make_range_m;
        use_pi_lower_bound_merge_sylls_complete_check_m;
    };

}

#endif
