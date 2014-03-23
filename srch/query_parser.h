#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <assert.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <boost/algorithm/string.hpp>
#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/query_tree.h>
#include <srch/special_search_params.h>
#include <srch/location_repos.h>
#include <srch/global_index.h>
#include <srch/itim_constants.h>

namespace itim {

class query_parser {
    public:
    query_parser(const config_branch &province_config, global_index &_index);
    void parse_params(const string& query, query_tree &qt, special_search_params &sp);

    private:
    typedef std::map<string, string> param_map;
    enum {OTHER_PHRASE = 0, QUOTE_PHRASE = 1, PLUS_PHRASE = 2};

    private:
    void build_tree(query_tree &tree, param_map &params);
    void build_special_search_params(special_search_params &sp, param_map &params);

    private:
    location_repository loc_repo;
    global_index* index;

    private:
    void parse_secondary_sites(query_tree& tree, param_map &params);
    void parse_locations(query_tree& tree, param_map &params);
    void parse_office_syll_ids(query_tree& tree, param_map &params);
    void parse_topic_ids(query_tree& tree, param_map &params);
    void parse_synonym_info(synonym_info& sinfo, const string& synonym_str);
    void parse_synonym_flags(synonym_info& sinfo, const string& flags);
    void parse_phrases(query_tree& tree, param_map &params);
    void parse_minus_phrases(query_tree& tree, param_map &params);
    void parse_tops(query_tree& tree, param_map &params);

    void calc_total_sylls(query_tree& tree);
    void calc_scan_info(query_tree& tree);
    void calc_scan_info_phrase(query_phrase& phrase, size_t& max_docid_num, token& token_);
};

}

#endif
