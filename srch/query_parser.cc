#include <srch/query_parser.h>

namespace itim {

query_parser::query_parser(const config_branch &province_config, global_index &_index) :
    loc_repo(province_config), index(&_index) {}

void query_parser::parse_params(const string& query, query_tree &qt, special_search_params &sp) {
    using namespace std;
    vector<string> pairs;
    param_map params;
    boost::split(pairs, query, boost::is_any_of("&"));
    for (auto i = pairs.begin(); i != pairs.end(); i++) {
        vector<string> parts;
        boost::split(parts, *i, boost::is_any_of("="));
        if ( parts.size() == 2 ) {
            params[parts[0]] = parts[1];
        } else if ( parts.size() == 1 ) {
            params[parts[0]] = "";
        } else {
            throw exception("Query parsing error: malformed param/value pair found. Too many equal signs? ");
        }
    }
    log_debug("query \"" << query << "\" parsed");
    build_tree(qt, params);
    build_special_search_params(sp, params);
}

void query_parser::build_tree(query_tree &tree, param_map &params) {
    tree.clear();

    if (params.count("qlang") == 1) {
        tree.vnese = (params.find("qlang")->second == "vn");
    }
    if (params.count("site") == 1) {
        tree.search_site = params.find("site")->second;
    }
    parse_secondary_sites(tree, params);
    parse_locations(tree, params);
    parse_office_syll_ids(tree, params);
    parse_topic_ids(tree, params);
    parse_phrases(tree, params);
    log_debug("plus_phrase.size() = " << tree.plus_phrase.size());
    log_debug_hard(tree.plus_phrase);
    log_debug("quote_phrase.size() = " << tree.quote_phrase.size());
    log_debug_hard(tree.quote_phrase);
    log_debug("other_phrase.size() = " << tree.other_phrase.size());
    log_debug_hard(tree.other_phrase);
    log_debug("main_group_id = " << ( tree.main_group_id == no_main_group_id ?
                                      "no_main_group_id" : "set" ) );
    parse_minus_phrases(tree, params);
    log_debug("minus_phrase.size() = " << tree.min_phrase.size());
    parse_tops(tree, params);
    calc_total_sylls(tree);
    calc_scan_info(tree);
    log_debug("query_parser tree built");
}

void query_parser::build_special_search_params(special_search_params &sp, param_map &params) {
    using namespace std;

    if (params.count("npruning") == 1) {
        sp.npruning = atoi(params.find("npruning")->second.c_str());
    } else {
        sp.npruning = default_draft_npruning;
    }

    if (params.count("ml") == 1) {
        sp.ml = true;
    } else {
        sp.ml = false;
    }

    if (params.count("textrank") == 1) {
        sp.textrank = true;
    } else {
        sp.textrank = false;
    }

    uint32_t metatimeout;
    if (params.count("metatimeout") == 1) {
        metatimeout = atoi(params.find("metatimeout")->second.c_str());
    } else {
        metatimeout = default_metatimeout;
    }

    sp.timeout = metatimeout - metatimeout_timeout_delta;
    sp.timeout_draft = metatimeout - metatimeout_timeout_draft_delta;

    if (params.count("clickinfo") == 1) {
        sp.use_click_info = ( atoi(params.find("metatimeout")->second.c_str()) == 1 );
    } else {
        sp.use_click_info = 1;
    }

    if (params.count("features") == 1) {
        sp.show_features = true;
    } else {
        sp.show_features = false;
    }

    if (params.count("notgroupbysite") == 1) {
        sp.not_group_by_site = true;
    } else {
        sp.not_group_by_site = false;
    }

    if (params.count("draftdebug") == 1) {
        sp.draft_debug = true;
    } else {
        sp.draft_debug = false;
    }

    if (params.count("userip") == 1) {
        sp.user_ip = ( params.find("userip")->second );
    } else {
        sp.user_ip = "";
    }

    if (params.count("preferprv") == 1) {
        sp.preferred_location = loc_repo.get(params.find("preferprv")->second);
    } else {
        sp.preferred_location = location();
    }

    if (params.count("fullappear") == 1) {
        sp.full_appearance = bool(atoi(params.find("fullappear")->second.c_str()));
    } else {
        sp.full_appearance = true;
    }
}

void query_parser::parse_secondary_sites(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("ss") == 1) {
        const string ssites_str = params.find("ss")->second;
        vector<string> ssites;
        boost::split(ssites, ssites_str, boost::is_any_of(","));
        for (auto i = ssites.begin(); i != ssites.end(); ++i) {
            siteid_t siteid = index->get_siteid(*i);
            if ( siteid != no_siteid ) {
                tree.secondary_site_ids.insert(siteid);
            }
        }
    }

    if (params.count("ess") == 1) {
        const string ess = params.find("ess")->second;
        tree.exact_secondary_site = (ess == "1");
    }
}

void query_parser::parse_locations(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("loc") == 1) {
        const string location_str = params.find("loc")->second;
        size_t delim = location_str.find_first_of(':');
        string left = location_str.substr(0, delim - 1);
        tree.location_level = atoi(left.c_str());
        string right = location_str.substr(delim + 1);;
        vector<string> loc_ids;
        boost::split(loc_ids, right, boost::is_any_of(","));
        for (auto i = loc_ids.begin(); i != loc_ids.end(); i++) {
            location loc = loc_repo.get_by_id(*i);
            if (loc) {
                tree.locs.push_back(loc);
            }
        }
    }
}

void query_parser::parse_office_syll_ids(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("osi") == 1) {
        const string osi_str = params.find("osi")->second;
        vector<string> osi;
        boost::split(osi, osi_str, boost::is_any_of(","));
        for (auto i = osi.begin(); i != osi.end(); i++) {
            if ( i->length() != 0)
                tree.office_syll_ids.push_back(atoi((*i).c_str()));
        }
    }
}

void query_parser::parse_topic_ids(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("tids") == 1) {
        const string tids_str = params.find("tids")->second;
        uint64_t tids = atol(tids_str.c_str());
        tree.topic_ids = tids;
    } else {
        tree.topic_ids = 0;
    }
}

void query_parser::parse_synonym_info(synonym_info& sinfo, const string& synonym_str) {
    using namespace std;

    vector<string> parts;
    boost::split(parts, synonym_str, boost::is_any_of(":"));

    if (parts.size() != 9) {
        throw exception("Query parsing error: synonym info string parts number is less than 9 ")
            << where(__PRETTY_FUNCTION__);
    }

    vector<string> pos_parts;
    boost::split(pos_parts, parts[2], boost::is_any_of("-"));
    sinfo.begin_token = atoi(pos_parts[0].c_str());
    sinfo.end_token = atoi(pos_parts[1].c_str());
    sinfo.score = atof(parts[8].c_str());
    sinfo.type = static_cast<synonym_info::synonym_info_type>( atoi(parts[1].c_str()) );

    vector<string> tokens;
    boost::split(tokens, parts[4], boost::is_any_of("-"));
    for (auto iter = tokens.begin(); iter != tokens.end(); iter++) {
        sinfo.push_back(token(*iter));
    }

    vector<string> non_tone_tokens;
    boost::split(non_tone_tokens, parts[5], boost::is_any_of("-"));
    for (auto iter = non_tone_tokens.begin(); iter != non_tone_tokens.end(); iter++) {
        sinfo.non_tone_tokens.push_back(token(*iter));
    }

    sinfo.vnese = (parts[0] == "vn");
    sinfo.idf = atof(parts[6].c_str()) / 1000;
    sinfo.basic_idf = atof(parts[7].c_str()) / 1000;

    parse_synonym_flags(sinfo, parts[3]);
}

void query_parser::parse_synonym_flags(synonym_info& sinfo, const string& flags) {
    for (size_t k = 0; k < flags.length(); ++k) {
        switch (flags[k]) {
        case 'b':
            sinfo.base = true;
            break;
        case 'N':
            sinfo.name = true;
            break;
        case 'S':
            sinfo.spec = true;
            break;
        case 'c':
            sinfo.conj = true;
            break;
        case 'd':
            sinfo.first_domain_level = true;
            break;
        case 'U':
            sinfo.urlsyn = true;
            break;
        case 'C':
            sinfo.city = true;
            break;
        default:
            break;
        }
    }
}

void query_parser::parse_phrases(query_tree& tree, param_map &params) {
    using namespace std;
    vector<string> sylls;
    if (params.count("sylls") == 1) {
        const string sylls_str = params.find("sylls")->second;
        boost::split(sylls, sylls_str, boost::is_any_of("|"));
    }
    vector<string> syns;
    if (params.count("syns") == 1) {
        const string syns_str = params.find("syns")->second;
        boost::split(syns, syns_str, boost::is_any_of("|"));
    }
    vector<string> msi;
    if (params.count("msi") == 1) {
        const string msi_str = params.find("msi")->second;
        boost::split(msi, msi_str, boost::is_any_of("|"));
    }
    vector<string> hostranges;
    if (params.count("hostranges") == 1) {
        const string hostranges_str = params.find("hostranges")->second;
        boost::split(hostranges, hostranges_str, boost::is_any_of("|"));
    }
    vector<string> group_infos;
    if (params.count("gi") == 1) {
        const string gi_str = params.find("gi")->second;
        boost::split(group_infos, gi_str, boost::is_any_of("|"));
    }

    if (syns.size() != sylls.size()) {
        throw exception("Query parsing error: synonym quantity is not equal to syllable quantity")
            << where(__PRETTY_FUNCTION__);
    }
    if ( !msi.empty() && msi.size() != sylls.size()) {
        throw exception("Query parsing error: msi quantity is not equal to syllable quantity")
            << where(__PRETTY_FUNCTION__);
    }
    if ( !hostranges.empty() != 0 && hostranges.size() != sylls.size()) {
        throw exception("Query parsing error: host ranges quantity is not equal to syllable quantity")
            << where(__PRETTY_FUNCTION__);
    }
    if (group_infos.size() != sylls.size() ) {
        throw exception("Query parsing error: group info quantity is not equal to syllable quantity")
            << where(__PRETTY_FUNCTION__);
    }

    // single group parsing
    for (size_t i = 0; i < syns.size(); ++i) {
        query_group qgroup;

        vector<string> syll;
        boost::split(syll, sylls[i], boost::is_any_of(" "));

        vector<string> main_syn;
        if ( !msi.empty() ) {
            boost::split(main_syn, msi[i], boost::is_any_of(","));
        }

        vector<string> hr;
        if ( !hr.empty() ) {
            boost::split(hr, hostranges[i], boost::is_any_of(","));
        }

        vector<string> syn;
        boost::split(syn, syns[i], boost::is_any_of(","));

        vector<string> info;
        boost::split(info, group_infos[i], boost::is_any_of(","));
        if (info.size() > 1) {
            qgroup.topic_synonym_off = atoi(info[1].c_str());
        }

        for (size_t j = 0; j < syll.size(); ++j) {
            vector<string> parts;
            boost::split(parts, syll[j], boost::is_any_of(":"));
            qgroup.syllable_infos.push_back(syllable_info(parts[0], atoi(parts[1].c_str()) / 1000.0));
        }

        if (main_syn.size() > 0) {
            for (size_t j = 0; j < main_syn.size(); j++) {
                qgroup.main_synonym_offs.push_back(atoi(main_syn[j].c_str()));
            }
            qgroup.is_main = true;
        }

        if (hr.size() > 0) {
            for (size_t j = 0; j < hr.size(); ++j) {
                if ( hr[j].length() != 0)
                    qgroup.host_ranges.insert(atoi(hr[j].c_str()));
            }
        }

        // synonym parsing
        for (size_t j = 0; j < syn.size(); j++) {
            synonym_info sinfo;
            parse_synonym_info(sinfo, syn[j]);
            qgroup.push_back(sinfo);
        }

        switch(atoi(info[0].c_str())) {
        case PLUS_PHRASE:
            tree.plus_phrase.push_back(qgroup);
            break;
        case QUOTE_PHRASE:
            tree.quote_phrase.push_back(qgroup);
            break;
        case OTHER_PHRASE:
            if ( qgroup.is_main ) {
                tree.main_group_id = tree.other_phrase.size();
                tree.weak_main_synonym = qgroup.is_weak_main_synonym();
            }
            tree.other_phrase.push_back(qgroup);
            break;
        default:
            throw exception("Query parsing error: unknown phrase type. ");
        }
    }
}

void query_parser::parse_minus_phrases(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("minus") == 1) {
        const string minus_str = params.find("minus")->second;
        vector<string> groups;
        boost::split(groups, minus_str, boost::is_any_of("|"));
        for (auto i = groups.begin(); i != groups.end(); i++) {
            tree.min_phrase.push_back(minus_item());
            boost::split(tree.min_phrase.back(), (*i), boost::is_any_of(","));
        }
    }
}

void query_parser::parse_tops(query_tree& tree, param_map &params) {
    using namespace std;
    if (params.count("tops") == 1) {
        const string tops_str = params.find("tops")->second;
        vector<string> tops;
        boost::split(tops, tops_str, boost::is_any_of("|"));
        for (auto i = tops.begin(); i != tops.end(); i++) {
            vector<string> top_str;
            boost::split(top_str, (*i), boost::is_any_of(":"));
            siteid_t siteid = index->get_siteid(top_str[0]);
            tree.top_site_weights[siteid] = atol(top_str[1].c_str());
        }
    }
}

void query_parser::calc_total_sylls(query_tree& tree) {
    for (auto i = tree.plus_phrase.begin(); i != tree.plus_phrase.end(); i++) {
        tree.total_sylls += (*i).syllable_infos.size();
    }
    for (auto i = tree.quote_phrase.begin(); i != tree.quote_phrase.end(); i++) {
        tree.total_sylls += (*i).syllable_infos.size();
    }
    for (auto i = tree.other_phrase.begin(); i != tree.other_phrase.end(); i++) {
        tree.total_sylls += (*i).syllable_infos.size();
    }
}

void query_parser::calc_scan_info(query_tree& tree) {
    size_t max_docid_num = 0;
    token token_;
    calc_scan_info_phrase(tree.plus_phrase, max_docid_num, token_);
    calc_scan_info_phrase(tree.quote_phrase, max_docid_num, token_);
    calc_scan_info_phrase(tree.other_phrase, max_docid_num, token_);
    tree.scan_info.num_docids = max_docid_num;
    tree.scan_info.token_ = token_;
}

void query_parser::calc_scan_info_phrase(query_phrase& phrase, size_t& max_docid_num, token& token_) {
    for ( auto phr_i = phrase.begin(); phr_i != phrase.end(); ++phr_i ) {
        for ( auto grp_i = phr_i->begin(); grp_i != phr_i->end(); ++grp_i ) {
            for (auto tok_i = grp_i->begin(); tok_i != grp_i->end(); ++tok_i) {
                std::vector<lexicon_item> lex_items = index->search_lexicon(tok_i->term);
                size_t docid_num = 0;
                for (auto lex_item_i = lex_items.begin(); lex_item_i != lex_items.end(); ++lex_item_i) {
                    docid_num += lex_item_i->df;
                }
                if ( docid_num > max_docid_num ) {
                    max_docid_num = docid_num;
                    token_ = *tok_i;
                }
            }
        }
    }
}

}
