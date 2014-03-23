#ifndef GLOBAL_INDEX_H
#define GLOBAL_INDEX_H

#include <vector>
#include <unordered_set>
#include <boost/ptr_container/ptr_vector.hpp>
#include <srch/index.h>
#include <srch/itim_vector.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/composite_posting_iterator.h>
#include <srch/thread_map.h>
#include <srch/lexicon.h>

namespace itim {

class global_index : boost::noncopyable
{
public:
    typedef boost::ptr_vector<index<> > composite_index;

    template<pass_type pt, depth d>
    struct range {
        typedef itim::iterator_range<composite_posting_iterator<global_index, pt, d> > value;
    };

public:
    global_index(const config_branch &index_cfg) {
        std::vector<config_branch> bucket_branches = index_cfg.branches("bucket");
        boost::mutex indexes_insertion;
        thread_map<std::vector<config_branch>::iterator>(
                bucket_branches.begin(), bucket_branches.end(),
                [&] (config_branch &cfg) -> void {
                    try {
                        log_debug("constructing index " << cfg.get<size_t>("id"));
                        index<>* ptr = new index<>(cfg);
                        log_debug("index " << cfg.get<size_t>("id") << " constructed");
                        boost::unique_lock<boost::mutex> indexes_insertion_lock(indexes_insertion);
                        indexes.push_back(ptr);
                    } catch(exception &e) {
                        log_critical("Exception: " << e.what() <<
                            ", file = " << (boost::get_error_info<itim::file>(e) ?
                                *boost::get_error_info<itim::file>(e) : "null") <<
                            ", where = " << (boost::get_error_info<itim::where>(e) ?
                                *boost::get_error_info<itim::where>(e) : "null"));
                    } catch(std::exception &e) {
                        log_critical("STD Exception: " << e.what());
                    } catch(...) {
                        log_critical("Unknown exception");
                    }
                }
            );
        /*
        for (auto i = bucket_branches.begin(); i != bucket_branches.end(); ++i) {
            log_debug("constructing index " << i->get<size_t>("id"));
            index<>* ptr = new index<>(*i);
            indexes.push_back(ptr);
        }
        */
        std::sort(indexes.c_array(), indexes.c_array() + indexes.size(),
                [&] (index<> *a, index<> *b) -> bool { return a->id < b->id; });
        log_debug("global index constructed, order:" <<
                [&] (composite_index &indexes) -> std::string {
                    std::ostringstream os;
                    for(size_t i = 0; i < indexes.size(); ++i) os << ' ' << i << ':' << indexes[i].id;
                    return os.str();
                } (indexes)
            );
    }

    web_page_info get_web_page_info(docid_t docid) {
        if ( docid.bucketid > indexes.size() ) {
            return web_page_info();
        }
        log_debug_mare("global_index[" << docid.bucketid << "].get_web_page_info[" << docid << "]");
        return indexes[docid.bucketid].wpis.get(docid);
    }

    siteid_t get_siteid(string domain) {
        for (auto i = indexes.begin(); i != indexes.end(); ++i) {
            siteid_t siteid = i->d2s.get(domain);
            if ( siteid != no_siteid ) {
                return siteid;
            }
        }
        return no_siteid;
    }

    bool is_spam(const docid_t& id) {
        return (spam_docid_set.count(id) == 1);
    }

private:
    typedef std::unordered_set<docid_t> spam_docid_set_t;
    spam_docid_set_t spam_docid_set;
    composite_index indexes;

public:
    template<pass_type ir, depth d>
    typename range<ir, d>::value posting_iterator_range(const term_type &term) {
        typename range<ir, d>::value r(term, indexes);
        log_debug_mare("global_index.posting_iterator_range(\"" << term << "\") = "
                << (r.empty() ? "empty" : "not empty"));
        return r;
    }

    std::vector<lexicon_item> search_lexicon(term_type term) {
        std::vector<lexicon_item> result;
        for (auto i = indexes.begin(); i != indexes.end(); ++i) {
            lexicon<> &lex = i->lexicon_ref();
            auto lex_iter = lex.find(term);
            if ( lex_iter != lex.end() ) {
                result.emplace_back( lex_iter->second );
            }
        }
        return result;
    }

};

}

#endif
