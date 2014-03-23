#ifndef QUERY_TREE_H
#define QUERY_TREE_H

#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <srch/itim_base.h>
#include <srch/location_repos.h>

namespace itim {

struct scan_info_t {
    size_t num_docids;
    token token_;
};

const size_t no_main_group_id = size_t(0) - 1;

struct query_tree {
    query_tree() { clear(); }

    void clear() {
        search_site.clear();
        vnese = true;
        locs.clear();
        plus_phrase.clear();
        quote_phrase.clear();
        other_phrase.clear();
        main_group_id = no_main_group_id;
        min_phrase.clear();
        secondary_site_ids.clear();
        office_syll_ids.clear();
        exact_secondary_site = false;
        top_site_weights.clear();
        total_sylls = 0;
        weak_main_synonym = false;
    }

    // parsed data
    string search_site;
    bool vnese;
    locations locs;
    uint16_t location_level;
    query_phrase plus_phrase;
    query_phrase quote_phrase;
    query_phrase other_phrase;
    size_t main_group_id;
    minus_phrase min_phrase;
    uint32_t topic_ids;
    std::set<siteid_t> secondary_site_ids;
    std::vector<size_t> office_syll_ids;
    bool exact_secondary_site;
    std::unordered_map<siteid_t, draft_score_t> top_site_weights;

    // calculated data
    size_t total_sylls;
    scan_info_t scan_info;
    bool weak_main_synonym;

};

}

#endif
