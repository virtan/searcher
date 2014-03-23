#ifndef PLUS_PHRASE_ITERATOR_H
#define PLUS_PHRASE_ITERATOR_H

#include <list>
#include <srch/itim_base.h>
#include <srch/phrase_iterator.h>
#include <srch/synonym_iterator.h>
#include <srch/itim_iterator_range.h>

namespace itim {

    // plus_group_iterator
    define_phrase_iterator_header(plus_group_iterator,
                           define_pi_subiterator(synonym_isec_iterator),
                           define_pi_source_type(query_group))
    {
        use_pi_legacty_type_m(plus_group_iterator);
        use_pi_empty_constructor_m(plus_group_iterator);
        use_pi_constructor_m(plus_group_iterator);
        use_pi_lower_bound_intersection_first_m;
        use_pi_fill_score_table_using_synonym_infos_m;
        use_pi_make_range_m;
    };

    // plus_phrase_iterator
    define_phrase_iterator_header(plus_phrase_iterator,
                           define_pi_subiterator(plus_group_iterator),
                           define_pi_source_type(query_phrase))
    {
        use_pi_legacty_type_m(plus_phrase_iterator);
        use_pi_empty_constructor_m(plus_phrase_iterator);
        use_pi_constructor_m(plus_phrase_iterator);
        use_pi_lower_bound_intersection_m;
        use_pi_fill_score_table_underlaying_fill_m;
        use_pi_make_range_m;
        use_pi_phrase_predicted_increment_m;
    };

}

#endif
