#ifndef SEARCHER_H
#define SEARCHER_H

#include <eger/timer.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/itim_algo.h>
#include <srch/query_tree.h>
#include <srch/search_tree_iterator.h>
#include <srch/special_search_params.h>
#include <srch/draft_search_results.h>
#include <srch/draft_score_calculator.h>
#include <srch/global_index.h>

namespace itim {

    class searcher {
        public:
        hit_list search_web(global_index &ind, query_tree &qtree,
                special_search_params &xparams) {
            log_debug("searching started");
            // TODO: search_statistics stat;
            hit_list hits = search_tier(ind, qtree, xparams);
            // TODO: stat.search_result(hits);
            log_debug("searching finished, hit list size = " << hits.size());
            return hits;
        }

        hit_list search_tier(global_index &ind, query_tree &qtree,
                special_search_params &xparams) {
            log_debug_hard("in search_tier");
            bool search_specific_site = !qtree.search_site.empty();
            size_t no_limit = 0;
            profiler_start(draft_collector_cumulative);
            draft_search_results dres(xparams.npruning,
                    search_specific_site ? no_limit : num_doc_per_site);
            profiler_stop(draft_collector_cumulative);
            log_debug_hard("draft_search_results constructed");
            profiler_start(draft_total_passes_cumulative);
            if(xparams.npruning > 0)
                process_draft(ind, qtree, xparams, dres);

            profiler_dump(draft_plus_iterator_cumulative);
            profiler_dump(draft_quote_iterator_cumulative);
            profiler_dump(draft_main_group_iterator_cumulative);
            profiler_dump(draft_minus_filter_cumulative);
            profiler_dump(draft_other_group_iterators_cumulative);
            profiler_dump(draft_search_host_filtering_cumulative);
            profiler_dump(draft_remaining_sylls_check_cumulative);

            profiler_dump(draft_filtering_cumulative);
            profiler_dump(draft_score_calculation_cumulative);
            profiler_dump(draft_scanned_calculation_cumulative);
            profiler_dump(draft_collector_cumulative);
            profiler_dump(draft_total_passes_cumulative);
            return hit_list();
            //return process_full(ind, qtree, xparams, dres);
        }

        void process_draft(global_index &ind, query_tree &qtree,
                special_search_params &xparams, draft_search_results &dres) {

            eger::timer timeout_timer;
            timeout_timer.deadline(xparams.timeout_draft);

            this->process_draft_cycle<>(ind, qtree, xparams, dres, timeout_timer);

            bool search_specific_site = !qtree.search_site.empty();
            profiler_start(draft_collector_cumulative);
            if(xparams.draft_debug)
                dres.collector.sort_debug(search_specific_site, ind);
            else
                dres.collector.sort(search_specific_site, ind);
            profiler_stop(draft_collector_cumulative);
        }

        template<depth depth_ = first_block, appearance appr = usual_appearance>
        void process_draft_cycle(global_index &ind, query_tree &qtree,
                special_search_params &xparams, draft_search_results &dres, eger::timer timeout_timer) {

            if(depth_ == all_blocks) return;

            log_debug("draft pass for depth = " << depth_ << ", appearance = " << appr);
            profiler_start(draft_one_pass_cumulative);

            size_t draft_counter = 0;
            dres.draft_depth = depth_;
            profiler_start(draft_collector_cumulative);
            dres.collector.clear();
            profiler_stop(draft_collector_cumulative);

            typedef itim::iterator_range<search_tree_iterator<draft, depth_, appr> > search_tree_range;

            double scanned_ratio = 0;

            {
                search_tree_range stree_range(qtree, ind, timeout_timer);

                for(;!stree_range.empty(); stree_range.pop_front()) {

                    profiler_start(draft_filtering_cumulative);
                    if(ind.is_spam(stree_range.front()))
                        continue;

                    const web_page_info wpi = ind.get_web_page_info(stree_range.front());
                    if(!wpi)
                        log_debug("no web_page_info for " << stree_range.front());

                    if(!is_in_preferred_location(
                                xparams.preferred_location, wpi)) {
                        log_debug_hard("not in preferred_location, skipping");
                        continue;
                    } else {
                        log_debug_mare("in preferred_location");
                    }
                    profiler_stop(draft_filtering_cumulative);

                    ++draft_counter;

                    profiler_start(draft_score_calculation_cumulative);
                    draft_score_t draft_score = stree_range.begin().score();
                    log_debug_mare("got draft_score: " << draft_score);

                    if(wpi) {
                        auto gsw_iter = qtree.top_site_weights.find(wpi->site_for_ranking_hash());
                        if(gsw_iter != qtree.top_site_weights.end()) {
                            log_debug_mare("top_site_weight found: " << gsw_iter->second);
                            draft_score += gsw_iter->second;
                        } else {
                            log_debug_mare("top_site_weight not found");
                        };
                    }
                    profiler_stop(draft_score_calculation_cumulative);

                    log_debug_mare("process_draft.next() = " << stree_range.front() <<
                                   " (draft_score = " << draft_score << ")");

                    if(draft_score > 0) {
                        profiler_start(draft_collector_cumulative);
                        dres.collector.push(wpi->siteid_for_ranking(),
                                draft_score_calculator::readjust(draft_score, qtree, wpi),
                                stree_range.front());
                        profiler_stop(draft_collector_cumulative);
                    }

                    if((draft_counter & 1023) == 0 && timeout_timer.deadline()) {
                        log_warning("timeout " << timeout_timer << " > " << xparams.timeout_draft <<
                                " after " << draft_counter << " iterations");
                        break;
                    }
                }

                profiler_start(draft_scanned_calculation_cumulative);
                size_t scanned = stree_range.begin().scanned();
                profiler_stop(draft_scanned_calculation_cumulative);
                scanned_ratio = qtree.scan_info.num_docids ? scanned / qtree.scan_info.num_docids : 1;
                if(scanned_ratio < 1) scanned_ratio = 1;
                log_debug_hard("process_draft.scanned status (scanned = " << scanned << ", ratio = " << scanned_ratio << ")");
                dres.estimate = draft_counter / (size_t) scanned_ratio;
                dres.elapsed = timeout_timer.current();

            }

            log_debug(depth_ << "," << appr << " gave " << dres.collector.size() << " results");
            log_debug_hard("next depth check: ((collector.size) " << dres.collector.size() <<
                    " < (npruning) " << xparams.npruning << ") && ((draft_counter) " << draft_counter <<
                    " < (npruning) " << xparams.npruning << ") && ((elapsed) " << dres.elapsed <<
                    " < (timeout, mcs) " << eger::microseconds(draft_timeout_mcs) << "), " <<
                    "full_appearance = " << (xparams.full_appearance ? "true" : "false") << " && " <<
                    "weak_main_synonym = " << (qtree.weak_main_synonym ? "true" : "false"));

            profiler_dump(draft_one_pass_cumulative);

            if(dres.collector.size() < xparams.npruning &&
                    draft_counter < xparams.npruning &&
                    dres.elapsed < eger::microseconds(draft_timeout_mcs))
                if(next_depth_tmpl(depth_) == top_blocks && xparams.full_appearance && qtree.weak_main_synonym)
                    process_draft_cycle<next_depth_tmpl(depth_), full_appearance>
                        (ind, qtree, xparams, dres, timeout_timer);
                else if(depth_ == top_blocks && appr == full_appearance)
                    process_draft_cycle<depth_, usual_appearance>
                        (ind, qtree, xparams, dres, timeout_timer);
                else
                    process_draft_cycle<next_depth_tmpl(depth_), usual_appearance>
                        (ind, qtree, xparams, dres, timeout_timer);
            else
                log_debug("enough draft pass produced, not going futher than {" << depth_ << "," << appr << "}");

        }
        
    };

}

#endif
