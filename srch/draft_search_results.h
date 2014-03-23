#ifndef DRAFT_SEARCH_RESULTS_H
#define DRAFT_SEARCH_RESULTS_H

#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <algorithm>
#include <srch/itim_base.h>

namespace itim {

    class draft_collector
    {
        protected:
        struct draft_collector_per_site_item {
            draft_collector_per_site_item(const draft_score_t &_score,
                    const docid_t &_docid) :
                score(_score), docid(_docid) {}
            bool operator<(const draft_collector_per_site_item &other) const {
                return score < other.score; }
            bool operator>(const draft_collector_per_site_item &other) const {
                return score > other.score; }
            bool operator==(const draft_collector_per_site_item &other) const {
                return score == other.score; }
            bool operator<=(const draft_collector_per_site_item &other) const {
                return score <= other.score; }
            bool operator>=(const draft_collector_per_site_item &other) const {
                return score >= other.score; }
            bool operator<(const draft_score_t &sc) const { return score < sc; }
            bool operator>(const draft_score_t &sc) const { return score > sc; }
            bool operator==(const draft_score_t &sc) const { return score == sc; }
            bool operator<=(const draft_score_t &sc) const { return score <= sc; }
            bool operator>=(const draft_score_t &sc) const { return score >= sc; }

            draft_score_t score;
            docid_t docid;
        };

        typedef std::priority_queue<draft_collector_per_site_item,
                std::vector<draft_collector_per_site_item>,
                std::greater<draft_collector_per_site_item> > draft_collector_per_site;

        struct draft_collector_item {
            draft_collector_item():
                siteid(no_siteid) {}
            draft_collector_item(const siteid_t &_siteid, 
                    const draft_collector_per_site &_per_site) :
                siteid(_siteid), per_site(_per_site) {}
            bool operator<(const draft_collector_item &other) const {
                return per_site.empty() ? !other.per_site.empty() :
                       other.per_site.empty() ? false :
                       per_site.top() < other.per_site.top(); }
            bool operator>(const draft_collector_item &other) const {
                return other.per_site.empty() ? !per_site.empty() :
                       per_site.empty() ? false :
                       per_site.top() > other.per_site.top(); }
            bool operator==(const draft_collector_item &other) const {
                return (per_site.empty() && other.per_site.empty()) || 
                       (!per_site.empty() && !other.per_site.empty() &&
                       per_site.top() == other.per_site.top()); }
            bool operator<=(const draft_collector_item &other) const {
                return !operator>(other); }
            bool operator>=(const draft_collector_item &other) const {
                return !operator<(other); }

            bool operator<(const draft_score_t &score) const {
                return per_site.empty() ? true : per_site.top() < score; }
            bool operator>(const draft_score_t &score) const {
                return per_site.empty() ? false : per_site.top() > score; }
            bool operator==(const draft_score_t &score) const {
                return !per_site.empty() && per_site.top() == score; }
            bool operator<=(const draft_score_t &score) const {
                return !operator>(score); }
            bool operator>=(const draft_score_t &score) const {
                return !operator<(score); }

            void swap(const draft_collector_item &other) const {
                std::swap(siteid, other.siteid);
                per_site.swap(other.per_site);
            }

            mutable siteid_t siteid;
            mutable draft_collector_per_site per_site;
        };

        typedef std::multiset<draft_collector_item> draft_collector_set;

        typedef std::unordered_map<siteid_t, draft_collector_set::iterator>
            draft_collector_siteid_map;

        typedef std::vector<docid_t> draft_docid_vector;

        public:
        draft_collector(size_t _max_size, size_t _max_docs_per_site) :
            max_size(_max_size),
            max_docs_per_site(_max_docs_per_site),
            the_size(0)
        {
            log_debug_hard("draft_collector construction start with max_size = " << max_size);
            dc_siteid_map.reserve(max_size);
            //dc_set.reserve(max_size);
            log_debug_hard("draft_collector constructed");
        }

        void push(siteid_t siteid, draft_score_t score, docid_t docid) {
            bool draft_collector_item_change = true;
            bool set_new_element = true;
            draft_collector_siteid_map::iterator i = dc_siteid_map.find(siteid);
            draft_collector_item new_item;
            if(i != dc_siteid_map.end()) {
                draft_collector_item &our_site = const_cast<draft_collector_item&>(*(i->second));
                if(max_docs_per_site &&
                        our_site.per_site.size() == max_docs_per_site) {
                    if(our_site >= score)
                        return;
                    our_site.per_site.pop();
                    set_new_element = false;
                } else {
                    if(our_site.per_site.size() && our_site <= score)
                        draft_collector_item_change = false;
                }
                if(set_new_element && the_size == max_size &&
                        *(dc_set.begin()) >= score)
                    return;
                our_site.per_site.push(draft_collector_per_site_item(score, docid));
                if(draft_collector_item_change) {
                    i->second->swap(new_item);
                    dc_set.erase(i->second);
                    dc_siteid_map.erase(i);
                }
            } else {
                if(the_size == max_size && *(dc_set.begin()) >= score)
                    return;
                new_item.siteid = siteid;
                new_item.per_site.push(
                        draft_collector_per_site_item(score, docid));
            }
            if(draft_collector_item_change)
                dc_siteid_map[siteid] = dc_set.insert(new_item);
            if(set_new_element)
                if(++the_size > max_size) {
                    draft_collector_set::iterator j = dc_set.begin();
                    j->swap(new_item);
                    new_item.per_site.pop();
                    dc_set.erase(j);
                    if(!new_item.per_site.empty()) dc_siteid_map[new_item.siteid] = dc_set.insert(new_item);
                    else dc_siteid_map.erase(new_item.siteid);
                    --the_size;
                }
        }

        size_t size() const { return the_size; }

        void clear() {
            dc_set.clear();
            dc_siteid_map.clear();
            dc_result.clear();
            the_size = 0;
        }

        void sort_debug(bool, global_index &) {
            // TODO
        }

        void sort(bool search_specific_site, global_index &ind) {
            bool max_docs_per_site_high_level = !search_specific_site &&
                size() > draft_size_high_level_limit;
            for(draft_collector_set::iterator i = dc_set.begin();
                    i!= dc_set.end(); ++i) {
                size_t new_max_docs_per_site = num_doc_per_site_high_level;
                while(max_docs_per_site_high_level && new_max_docs_per_site &&
                        i->per_site.size() > new_max_docs_per_site) {
                    if(ind.get_web_page_info(i->per_site.top().docid)->mainpage()) {
                        dc_result.emplace_back(i->per_site.top().docid);
                        --new_max_docs_per_site;
                    }
                    i->per_site.pop();
                }
                while(i->per_site.size()) {
                    dc_result.emplace_back(i->per_site.top().docid);
                    i->per_site.pop();
                }
            }
            sort_result();
        }

        private:
        void sort_result() {
            std::sort(dc_result.begin(), dc_result.end());
        }

        private:
        size_t max_size;
        size_t max_docs_per_site;
        size_t the_size;
        draft_collector_set dc_set;
        draft_collector_siteid_map dc_siteid_map;
        draft_docid_vector dc_result;
    };

    class draft_search_results {
        public:
        draft_search_results(size_t max_size, size_t max_docs_per_site) :
            estimate(0), draft_depth(first_block), elapsed(0),
            collector(max_size, max_docs_per_site) {}
        size_t estimate;
        depth draft_depth;
        size_t elapsed;
        draft_collector collector;
    };

}

#endif
