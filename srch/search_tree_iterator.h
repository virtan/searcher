#ifndef SEARCH_TREE_ITERATOR_H
#define SEARCH_TREE_ITERATOR_H

#include <boost/iterator/iterator_facade.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <eger/timer.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/itim_algo.h>
#include <srch/global_index.h>
#include <srch/minus_filter.h>
#include <srch/plus_phrase_iterator.h>
#include <srch/quote_phrase_iterator.h>
#include <srch/other_phrase_iterator.h>

namespace itim {

    template<pass_type ptype, depth depth_, appearance appr, class index_type>
    struct search_tree_iterator_sub_iterators;

    template<pass_type ptype, depth depth_, class index_type>
    struct search_tree_iterator_sub_iterators<ptype, depth_, usual_appearance, index_type> {
        typedef main_group_iterator<ptype, depth_, index_type> main_group_iterator_type;
        typedef other_group_iterator<ptype, depth_, index_type> other_group_iterator_type;
    };

    template<pass_type ptype, depth depth_, class index_type>
    struct search_tree_iterator_sub_iterators<ptype, depth_, full_appearance, index_type> {
        typedef fa_main_group_iterator<ptype, depth_, index_type> main_group_iterator_type;
        typedef fa_other_group_iterator<ptype, depth_, index_type> other_group_iterator_type;
    };

    template<pass_type ptype, depth depth_, appearance appr = usual_appearance>
    class search_tree_iterator :
        public boost::iterator_facade<
            search_tree_iterator<ptype, depth_>,
            const docid_t&,
            boost::forward_traversal_tag>
    {
        public:
        typedef global_index index_type;

        typedef plus_phrase_iterator<ptype, depth_, index_type> plus_phrase_iterator_type;
        typedef itim::iterator_range<plus_phrase_iterator_type> plus_phrase_range;

        typedef quote_phrase_iterator<ptype, depth_, index_type> quote_phrase_iterator_type;
        typedef itim::iterator_range<quote_phrase_iterator_type> quote_phrase_range;

        typedef search_tree_iterator_sub_iterators<ptype, depth_, appr, index_type> sub_iterators;

        typedef typename sub_iterators::main_group_iterator_type main_group_iterator_type;
        typedef itim::iterator_range<main_group_iterator_type> main_group_range;

        typedef typename sub_iterators::other_group_iterator_type other_group_iterator_type;
        typedef itim::iterator_range<other_group_iterator_type> other_group_range;
        typedef std::vector<other_group_range> other_group_ranges;

        typedef minus_filter<ptype, depth_, index_type> minus_filter_type;

        public:
        search_tree_iterator() :
            current_docid(no_docid),
            end_flag(true),
            search_site_parsed(false)
        {}

        search_tree_iterator(query_tree &_qtree, index_type &_ind, eger::timer _timeout) :
            qtree(&_qtree),
            ind(&_ind),
            timeout(_timeout),
            current_docid(no_docid),
            end_flag(false),
            //plus_range(plus_phrase_iterator_type(_ind, qtree->plus_phrase, timeout), plus_phrase_iterator_type()),
            //quote_range(quote_phrase_iterator_type(_ind, qtree->quote_phrase, timeout), quote_phrase_iterator_type()),
            //main_gr_range(qtree->main_group_id != no_main_group_id ?
            //            main_group_iterator_type(_ind, qtree->other_phrase[qtree->main_group_id], timeout) :
            //            main_group_iterator_type(),
            //        main_group_iterator_type()),
            //minus_filt(_ind, qtree->min_phrase),
            search_site_parsed(false),
            minimal_syll_appearance(std::max((size_t) 1, qtree->total_sylls / 2))
        {
            profiler_start(draft_plus_iterator_cumulative);
            plus_range.begin() = plus_phrase_iterator_type(_ind, qtree->plus_phrase, timeout);
            profiler_stop(draft_plus_iterator_cumulative);
            profiler_start(draft_quote_iterator_cumulative);
            quote_range.begin() = quote_phrase_iterator_type(_ind, qtree->quote_phrase, timeout);
            profiler_stop(draft_quote_iterator_cumulative);
            profiler_start(draft_main_group_iterator_cumulative);
            if(qtree->main_group_id != no_main_group_id)
                main_gr_range.begin() =
                    main_group_iterator_type(_ind, qtree->other_phrase[qtree->main_group_id], timeout);
            profiler_stop(draft_main_group_iterator_cumulative);
            profiler_start(draft_minus_filter_cumulative);
            minus_filt = minus_filter_type(_ind, qtree->min_phrase);
            profiler_stop(draft_minus_filter_cumulative);
            profiler_start(draft_other_group_iterators_cumulative);
            other_gr_ranges.reserve(qtree->other_phrase.size());
            for(size_t i = 0; i < qtree->other_phrase.size(); ++i) {
                if(i == qtree->main_group_id) continue;
                other_gr_ranges.emplace_back(
                        other_group_range(other_group_iterator_type(*ind, qtree->other_phrase[i], timeout),
                            other_group_iterator_type()));
            }
            profiler_stop(draft_other_group_iterators_cumulative);
            fill_max_remaining_sylls();
            parse_search_site();
            increment();
        }

        public:
        search_tree_iterator(const search_tree_iterator&) { assert(false); }
        search_tree_iterator &operator=(const search_tree_iterator&) { assert(false); return *this; }

        private:
        void fill_max_remaining_sylls() {
            size_t total_query_groups = qtree->plus_phrase.size() + qtree->quote_phrase.size() +
                qtree->other_phrase.size();
            max_remaining_sylls.resize(total_query_groups);
            size_t remaining_sylls = 0;
            for(size_t ri = qtree->other_phrase.size(); ri > 0;) {
                if(--ri == qtree->main_group_id) continue;
                max_remaining_sylls[--total_query_groups] = remaining_sylls;
                remaining_sylls += qtree->other_phrase[ri].syllable_infos.size();
            }
            if(qtree->main_group_id != no_main_group_id) {
                max_remaining_sylls[--total_query_groups] = remaining_sylls;
                remaining_sylls += qtree->other_phrase[qtree->main_group_id].syllable_infos.size();
            }
            for(query_phrase::reverse_iterator ri = qtree->quote_phrase.rbegin();
                    ri != qtree->quote_phrase.rend(); ++ri) {
                max_remaining_sylls[--total_query_groups] = remaining_sylls;
                remaining_sylls += ri->syllable_infos.size();
            }
            for(query_phrase::reverse_iterator ri = qtree->plus_phrase.rbegin();
                    ri != qtree->plus_phrase.rend(); ++ri) {
                max_remaining_sylls[--total_query_groups] = remaining_sylls;
                remaining_sylls += ri->syllable_infos.size();
            }
            if(total_query_groups)
                max_remaining_sylls[--total_query_groups] = remaining_sylls;
        }

        bool enough_remaining_sylls(size_t syll_count, size_t current_query_group) {
            assert(current_query_group < max_remaining_sylls.size());
            bool enough = (syll_count + max_remaining_sylls[current_query_group]) >= minimal_syll_appearance;
            if(!enough) {
                log_debug_mare("search_tree_iterator.enough_remaining_sylls() = " <<
                        (syll_count + this->max_remaining_sylls[current_query_group]) << " < " <<
                        this->minimal_syll_appearance);
            }
            return enough;
        }

        private:
        friend class boost::iterator_core_access;

        const docid_t &dereference() const {
            log_debug_mare("search_tree_iterator[" << depth_ << "," << appr << "].dereference() = " << this->current_docid );
            return current_docid;
        }

        void increment() {
            if(end_flag) return;
            this->lower_bound(current_docid == no_docid ? docid_t(0, 0) : current_docid + 1);
        }

        public:
        void lower_bound(docid_t min_docid) {
            docid_t __attribute__((unused)) _min_docid = min_docid;

            while(!end_flag) {

                current_docid = no_docid;
                if(!plus_range.empty()) {
                    profiler_start(draft_plus_iterator_cumulative);
                    current_docid = lower_bound_return(plus_range, min_docid, current_docid);
                    profiler_stop(draft_plus_iterator_cumulative);
                } else if(!quote_range.empty()) {
                    profiler_start(draft_quote_iterator_cumulative);
                    current_docid = lower_bound_return(quote_range, min_docid, current_docid);
                    profiler_stop(draft_quote_iterator_cumulative);
                } else {
                    profiler_start(draft_main_group_iterator_cumulative);
                    current_docid = lower_bound_return(main_gr_range, min_docid, current_docid);
                    profiler_stop(draft_main_group_iterator_cumulative);
                }

                if (current_docid == no_docid) {
                    end_flag = true;
                    continue;
                }

                profiler_start(draft_minus_filter_cumulative);
                bool minus_filtered = !minus_filt(current_docid);
                profiler_stop(draft_minus_filter_cumulative);
                if(minus_filtered) {
                    log_debug_mare("stree_range.lower_bound().minus_filt(" << this->current_docid
                            << ") = minus_filtered (false), skipping");
                    min_docid = current_docid + 1;
                    continue;
                }

                if(!plus_range.empty() && !quote_range.empty()) {
                    docid_t quote_docid = no_docid;
                    profiler_start(draft_quote_iterator_cumulative);
                    quote_docid = lower_bound_return(quote_range, current_docid, quote_docid);
                    profiler_stop(draft_quote_iterator_cumulative);
                    if(quote_docid == no_docid) {
                        log_debug_mare("stree_range.lower_bound().quote_range(" << this->current_docid
                                << ") = not found, skipping");
                        current_docid == no_docid;
                        end_flag = true;
                        continue;
                    }
                    if(quote_docid > current_docid) {
                        log_debug_mare("stree_range.lower_bound().quote_range(" << this->current_docid
                                << ") = not found, skipping to " << quote_docid);
                        min_docid = quote_docid;
                        continue;
                    }
                }

                profiler_start(draft_search_host_filtering_cumulative);
                if(search_host.size() > 0 && ((search_home_page &&
                                !ind->get_web_page_info(current_docid)->mainpage()
                                && ind->get_web_page_info(current_docid)->page_priority() < 1 - search_tree_eps)
                            || !is_doc_for_search_site(ind->get_web_page_info(current_docid)->url()))) {
                    profiler_stop(draft_search_host_filtering_cumulative);
                    log_debug_mare("stree_range.lower_bound().search_host(" << this->current_docid
                            << ") = out, skipping");
                    min_docid = current_docid + 1;
                    continue;
                }
                profiler_stop(draft_search_host_filtering_cumulative);

                profiler_start(draft_remaining_sylls_check_cumulative);
                size_t syll_passed = 0;
                if(!check_remaining_sylls(syll_passed)) {
                    min_docid = predict_next(syll_passed, min_docid);
                    profiler_stop(draft_remaining_sylls_check_cumulative);
                    log_debug_mare("stree_range.lower_bound().enough_remaining_sylls("
                            << this->current_docid << ") = not_enough, skipping to " << min_docid);
                    continue;
                }
                profiler_stop(draft_remaining_sylls_check_cumulative);

                break;
            }

            log_debug_mare("stree_range.lower_bound(" << _min_docid << ") = " << this->current_docid);

        }

        bool equal(search_tree_iterator const &other) const {
            if(end_flag || other.end_flag)
                return end_flag == other.end_flag;
            return current_docid == other.current_docid; // may be insufficient
        }

        size_t scanned() const {
            size_t scnd = 0;
            scnd += plus_range.begin().scanned();
            scnd += quote_range.begin().scanned();
            scnd += main_gr_range.begin().scanned();
            for(typename other_group_ranges::const_iterator i = other_gr_ranges.begin();
                    i != other_gr_ranges.end(); ++i)
                scnd += i->begin().scanned();
            return scnd;
        }

        draft_score_t score() {
            return this->score(current_docid);
        }

        draft_score_t score(const docid_t &docid) {
            if(current_docid != docid)
                return draft_score_t();
            syll_draft_score_t syll_draft_score;
            syll_draft_score.resize(qtree->total_sylls);
            if(!plus_range.empty())
                plus_range.begin().fill_score_table(docid, syll_draft_score);
            if(!quote_range.empty())
                quote_range.begin().fill_score_table(docid, syll_draft_score);
            if(!main_gr_range.empty())
                main_gr_range.begin().fill_score_table(docid, syll_draft_score);
            for(typename other_group_ranges::iterator i = other_gr_ranges.begin(); i != other_gr_ranges.end(); ++i) {
                if(i->empty()) continue;
                i->begin().fill_score_table(docid, syll_draft_score);
            }
            draft_score_t total_score(0);
            size_t sylls = 0;
            for(syll_draft_score_t::iterator i = syll_draft_score.begin(); i != syll_draft_score.end(); ++i)
                if(*i > 0) {
                    total_score += *i;
                    ++sylls;
                }
            return total_score * sylls > java_max_int ? java_max_int : total_score;
        }

        private:
        bool check_remaining_sylls(size_t &syll_passed) {
            size_t syll_count = 0;

            for(query_phrase::iterator i = qtree->plus_phrase.begin();
                    i != qtree->plus_phrase.end();
                    ++i, ++syll_passed) {
                syll_count += i->syllable_infos.size();
                if(!enough_remaining_sylls(syll_count, syll_passed))
                    return false;
            }

            if(!quote_range.empty()) {
                syll_count += quote_range.begin().summ_of_subappearances();
                syll_passed += qtree->quote_phrase.size();
                if(!enough_remaining_sylls(syll_count, syll_passed - 1))
                    return false;
            }

            docid_t candidate = no_docid;
            if(!main_gr_range.empty()) {
                candidate = lower_bound_return(main_gr_range, current_docid, candidate);
                if(candidate != no_docid && candidate == current_docid)
                    syll_count += main_gr_range.begin().appearance();
                if(!enough_remaining_sylls(syll_count, syll_passed++))
                    return false;
            }

            for(typename other_group_ranges::iterator i = other_gr_ranges.begin();
                    i != other_gr_ranges.end();
                    ++i) {
                candidate = no_docid;
                candidate = lower_bound_return(*i, current_docid, candidate);
                if(candidate != no_docid && candidate == current_docid)
                    syll_count += i->begin().appearance();
                if(!enough_remaining_sylls(syll_count, syll_passed++))
                    return false;
            }

            return true;
        }

        docid_t predict_next(size_t syll_passed, docid_t &current_min) {
            if(appr == full_appearance) return current_docid + 1;

            docid_t predicted = no_docid;

            if(!plus_range.empty())
                plus_range.begin().predicted_increment(syll_passed, current_min, predicted);
            if(!quote_range.empty())
                quote_range.begin().predicted_increment(syll_passed, current_min, predicted);
            if(!main_gr_range.empty())
                main_gr_range.begin().predicted_increment(syll_passed, current_min, predicted);
            for(typename other_group_ranges::const_iterator i = other_gr_ranges.begin();
                    syll_passed && i != other_gr_ranges.end(); ++i)
                if(!i->empty())
                    i->begin().predicted_increment(syll_passed, current_min, predicted);

            return (predicted != no_docid) && (predicted > current_min) ? predicted : current_min + 1;
        }

        void parse_search_site() {
            if(search_site_parsed) return;
            search_home_page = search_priority = search_head = search_end = false;
            string &search_site = qtree->search_site;
            size_t b = 0;
            for(bool beg_flags = true; beg_flags; ++b) {
                switch(b) {
                    case ':': search_home_page = true; break;
                    case '+': search_priority = true; break;
                    case '|': search_head = true; break;
                    default: beg_flags = false; break;
                }
            }
            --b;
            size_t e;
            for(e = b; e < search_site.size() && search_site[e] != '/'; ++e);
            if(e < search_site.size()) {
                if(e > b) search_host.append(search_site, b, e - b);
                b = e + 1;
                search_end = search_site[search_site.size() - 1] == '/';
                e = search_end ? search_site.size() - 1 : search_site.size();
                if(e > b) search_tail.append(search_site, b, e - b);
            }
            search_site_parsed = true;
        }

        bool is_doc_for_search_site(const string &url) {
            size_t p = url.find('.');
            p = p == string::npos ? url.find('/') : url.find('/', p + 1);
            if(p == string::npos) {
                return search_tail.empty() ? is_doc_for_search_host(url) : false;
            }
            if(!is_doc_for_search_host(url, p))
                return false;
            if(!search_tail.size())
                return true;
            if(url.size() - p - 1 < search_tail.size() ||
                    icompare(
                        string(url.begin() + p + 1, url.begin() + p + 1 + search_tail.size()),
                        search_tail))
                return false;
            return !search_end || url.size() - p - 1 == search_tail.size() ||
                url[p + 1 + search_tail.size()] == '/';
        }

        bool is_doc_for_search_host(const string &host, string::size_type size = string::npos) {
            if(size == string::npos) size = host.size();
            if(size < search_host.size() ||
                    icompare(
                        string(host.begin() + size - search_host.size(), host.begin() + size),
                        search_host))
                return false;
            if(size == search_host.size() || search_host[0] == '.') return true;
            size_t p = size - search_host.size() - 1;
            switch(host[p]) {
                case '/': return true;
                case '.': break;
                default: return false;
            }
            if(p == 0 || !search_head) return true;
            size_t p2 = host.rfind('/', p - 1);
            p2 = p2 == string::npos ? 0 : p2 + 1;
            if(p - p2 < 3 || p - p2 > 5 ||
                    icompare(
                        string(host.begin() + p2, host.begin() + p2 + 3),
                        string("www")))
                return false;
            for(size_t i = p2 + 3; i < p; ++i)
                if(!isdigit(host[i]))
                    return false;
            return true;
        }

        private:
        query_tree *qtree;
        index_type *ind;
        eger::timer timeout;
        docid_t current_docid;
        bool end_flag;
        plus_phrase_range plus_range;
        quote_phrase_range quote_range;
        main_group_range main_gr_range;
        other_group_ranges other_gr_ranges;
        minus_filter_type minus_filt;
        bool search_site_parsed;
        bool search_home_page;
        bool search_priority;
        bool search_head;
        bool search_end;
        string search_host;
        string search_tail;
        std::vector<size_t> max_remaining_sylls;
        size_t minimal_syll_appearance;
    };

}

#endif
