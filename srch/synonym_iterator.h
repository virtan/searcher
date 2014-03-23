#ifndef SYNONYM_ITERATOR_H
#define SYNONYM_ITERATOR_H

#include <list>
#include <srch/itim_base.h>
#include <srch/phrase_iterator.h>

namespace itim {

    // synonym intersection iterator
    define_phrase_iterator_header(synonym_isec_iterator,
                           define_pi_subiterator_index_range,
                           define_pi_source_type(synonym_info))
    {
        use_pi_legacty_type_m(synonym_isec_iterator);
        use_pi_empty_constructor_m(synonym_isec_iterator);
        use_pi_constructor_m(synonym_isec_iterator);
        use_pi_posting_iterator_range_m;
        use_pi_lower_bound_intersection_m;
        use_pi_score_synonym_info_m;
    };

    // synonym merge iterator
    define_phrase_iterator_header(synonym_merge_iterator,
                           define_pi_subiterator_index_range,
                           define_pi_source_type(synonym_info))
    {
        use_pi_legacty_type_m(synonym_merge_iterator);
        use_pi_empty_constructor_m(synonym_merge_iterator);
        use_pi_constructor_m(synonym_merge_iterator);
        use_pi_posting_iterator_range_m;
        use_pi_lower_bound_merge_m;
        use_pi_score_synonym_info_m;
    };

}

#endif
