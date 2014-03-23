#ifndef SPECIAL_SEARCH_PARAMS_H
#define SPECIAL_SEARCH_PARAMS_H

#include <list>
#include <srch/itim_base.h>
#include <srch/location_repos.h>

namespace itim {

struct special_search_params {
    size_t npruning;
    bool ml;
    bool textrank;
    uint32_t timeout;
    uint32_t timeout_draft;
    bool use_click_info;
    bool show_features;
    bool not_group_by_site;
    bool draft_debug;
    location preferred_location;
    string user_ip;
    bool full_appearance;
    // TODO: what is that?!
    // std::list<docid_t> top_docids;
};

}

#endif
