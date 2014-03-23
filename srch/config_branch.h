#ifndef CONFIG_BRANCH_H
#define CONFIG_BRANCH_H

#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <srch/itim_base.h>

namespace itim {

class config_branch;

std::ostream &operator<<(std::ostream &, config_branch &);

class config_branch {

    public:
    config_branch(const string &_config_path) :
        config_path(_config_path) {
            load();
            expand(pt);
            log_debug_hard_multiline("expanded config\n" << *this);
        }

    config_branch(const boost::property_tree::ptree &b) :
        pt(b) {}

    private:
    void load() {
        if(config_path.empty())
            throw exception("Loading from empty config path") <<
                where(__PRETTY_FUNCTION__) <<
                file(config_path);
        try {
            read_json(config_path, pt);
        } catch(boost::property_tree::ptree_error &e) {
            throw exception(e.what()) <<
                where(__PRETTY_FUNCTION__) <<
                errno_code(errno) <<
                file(config_path);
        }
        log_debug("config \"" << config_path << "\" loaded");
    }

    void expand(boost::property_tree::ptree &pti) {
        for(auto i = pti.ordered_begin(); i != pti.not_found(); ++i) {
            if(i->first == "index_buckets") expand_index_buckets(i->second);
            expand(i->second);
        }
    }

    void expand_index_buckets(boost::property_tree::ptree &pti) {
        using namespace boost::filesystem;
        using namespace boost::property_tree;
        using namespace boost::assign;

        auto ind_i = pti.find("path");
        if(ind_i == pti.not_found()) return;
        string base_path = ind_i->second.data() + "/";

        boost::filesystem::path index_dir(base_path);
        std::vector<string> components = list_of
            ("index")("lexicon")("webpageinfo")("domain2site");
        if(is_directory(index_dir))
            for(auto i = directory_iterator(index_dir); i != directory_iterator(); ++i)
                if(is_directory(*i)) {
                    ptree current;
                    current.put("id", i->path().filename().c_str());
                    for(auto j = components.begin(); j != components.end(); ++j) {
                        ind_i = pti.find(*j);
                        if(ind_i == pti.not_found()) continue;
                        ptree new_component(ind_i->second);
                        ind_i = new_component.find("file_suffix");
                        new_component.put("file", base_path + i->path().filename().c_str() + "/" +
                                    (ind_i != new_component.not_found() ? ind_i->second.data() : ""));
                        new_component.erase("file_suffix");
                        current.put_child(*j, new_component);
                    }
                    pti.add_child("bucket", current);
                }

        pti.erase("path");
        for(auto i = components.begin(); i != components.end(); ++i) pti.erase(*i);
    }

    public:
    config_branch branch(const string &path) const {
        try {
            return config_branch(pt.get_child(path));
        } catch(boost::property_tree::ptree_error &e) {
            throw exception(e.what()) <<
                where(__PRETTY_FUNCTION__);
        }
    }

    std::vector<config_branch> branches(const string &path) const {
        string child_name;
        boost::property_tree::ptree target_pt;
        size_t rpos = path.rfind('.');
        if ( rpos != string::npos ) {
            string search_path = path.substr(0, rpos);
            child_name = path.substr(rpos+1);
            target_pt = pt.get_child(search_path);
        } else {
            child_name = path;
            target_pt = pt;
        }

        std::vector<config_branch> branches;
        try {
            auto range = target_pt.equal_range(child_name);
            for (auto i = range.first; i != range.second; i++) {
                branches.emplace_back(i->second);
            }
        } catch(boost::property_tree::ptree_error &e) {
            throw exception(e.what()) << where(__PRETTY_FUNCTION__);
        }
        return branches;
    }

    template <typename Type>
    Type get(const string &path) const {
        return pt.get<Type>(path, Type());
    }

    bool exist(const string &path) const {
        return pt.find(path) != pt.not_found();
    }

    bool check_boolean(const string &path) const {
        return exist(path) && get<string>(path) != "off";
    }

    private:
    friend std::ostream &operator<<(std::ostream &, config_branch &);
    string config_path;
    boost::property_tree::ptree pt;
};

inline void ostream_config_branch_level(std::ostream &s, boost::property_tree::ptree &pti, size_t level) {
    bool first = true;
    for(auto i = pti.ordered_begin(); i != pti.not_found(); ++i) {
        if(!first) s << ",\n";
        s << string(level * 4, ' ');
        s << i->first;
        s << ": ";
        if(i->second.empty()) {
            s << i->second.data();
        } else {
            s << '{';
            bool two_n_more = !(i->second.size() == 1 && i->second.begin()->second.empty());
            s << (two_n_more ? '\n' : ' ');
            ostream_config_branch_level(s, i->second, two_n_more ? level + 1 : 0 );
            s << string(two_n_more ? level * 4 : 0, ' ');
            s << "}";
        }
        first = false;
    }
    s << (pti.size() == 1 && pti.begin()->second.empty() ? ' ' : '\n');
}

inline std::ostream &operator<<(std::ostream &s, config_branch &cfg) {
    s << "{\n";
    ostream_config_branch_level(s, cfg.pt, 1);
    s << "}";
    return s;
}

}

#endif
