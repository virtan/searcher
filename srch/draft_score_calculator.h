#ifndef DRAFT_SCORE_CALCULATOR_H
#define DRAFT_SCORE_CALCULATOR_H

#include <srch/itim_base.h>
#include <srch/query_tree.h>
#include <srch/web_page_info.h>

namespace itim {

    class draft_score_calculator {
        public:
        static
        draft_score_t readjust(draft_score_t draft_score, query_tree &qtree,
                const web_page_info &wpi) {

            if(qtree.secondary_site_ids.find(wpi->siteid_for_ranking()) !=
                    qtree.secondary_site_ids.end())
                draft_score *= search_site_draft_coeff;

            if (wpi->mainpage())
                draft_score *= home_page_draft_coeff;

            draft_score /= 100;

            if (draft_score > draft_score_maximum - 1000000) {
                draft_score = draft_score_maximum - 1000000 +
                    (draft_score * 10 / draft_score_maximum);
                if (draft_score > draft_score_maximum)
                    draft_score = draft_score_maximum;
            }

            return draft_score;
        }
    };

}

#endif
