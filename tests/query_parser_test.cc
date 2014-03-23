#include <iostream>
#include <eger/logger.h>
#include <srch/global_index.h>
#include <srch/query_parser.h>

class dumper {
public:
    void operator()(const itim::query_tree& qt) {
        using namespace std;
        cout << "{ QTREE " << endl;
        cout << "search_site: " << qt.search_site << endl;
        cout << "vnese: " << qt.vnese << endl;
        cout << "locations: " << endl;
        (*this)(qt.locs);
        cout << "plus_phrase:" << endl;
        (*this)(qt.plus_phrase);
        cout << "quote_phrase:" << endl;
        (*this)(qt.quote_phrase);
        cout << "other_phrase:" << endl;
        (*this)(qt.other_phrase);
        cout << "min_phrase:" << endl;
        (*this)(qt.min_phrase);
        cout << "topic_ids:" << qt.topic_ids << endl;
        cout << "secondary_site_ids:" << endl;
        (*this)(qt.secondary_site_ids);
        cout << "office_syll_ids:" << endl;
        (*this)(qt.office_syll_ids);
        cout << "top_site_weights:" << endl;
        (*this)(qt.top_site_weights);
        cout << " }" << endl;
    }
    void operator()(const itim::locations& locs) {
        using namespace std;
        for ( auto i = locs.begin(); i != locs.end(); ++i ) {
            cout << "  " << i->id << endl;
            cout << "  " << i->full_name << endl;
        }
    }
    void operator()(const itim::query_phrase& phrase) {
        using namespace std;
        for ( auto i = phrase.begin(); i != phrase.end(); ++i ) {
            cout << "  " << "qgroup:" << endl;
            for (auto ii = i->begin(); ii != i->end(); ++ii) {
                cout << "    " << "syn: ";
                for (auto iii = ii->begin(); iii != ii->end(); ++iii) {
                    cout << iii->term << ", ";
                }
                cout << endl;
            }
        }
    }
    void operator()(const itim::minus_phrase& phrase) {
        using namespace std;
        for ( auto i = phrase.begin(); i != phrase.end(); ++i ) {
            cout << "  " << "minus item:" << endl;
            for (auto ii = i->begin(); ii != i->end(); ++ii) {
                cout << "    " << "term: ";
                cout << *ii << endl;
            }
        }
    }
    void operator()(const std::set<itim::siteid_t>& ssis) {
        using namespace std;
        for ( auto i = ssis.begin(); i != ssis.end(); ++i ) {
            cout << "  " << *i << endl;
        }
    }
    void operator()(const std::vector<size_t>& int_vector) {
        using namespace std;
        for ( auto i = int_vector.begin(); i != int_vector.end(); ++i ) {
            cout << "  " << *i << endl;
        }
    }
    void operator()(std::unordered_map<itim::siteid_t, itim::draft_score_t> weights) {
        using namespace std;
        for ( auto i = weights.begin(); i != weights.end(); ++i) {
            cout << "  " << i->first << " --> " << i->second << endl;
        }
    }
};

int main(int argc, char** argv) {
    eger::instance eger_logger;
    eger_logger[(size_t) eger::level_debug] = "stderr";
    eger_logger[(size_t) eger::level_critical] = "stderr";
    eger_logger[(size_t) eger::level_error] = "stderr";
    eger_logger[(size_t) eger::level_warning] = "stderr";
    eger_logger.start_sync_writer();

    using namespace std;
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <query>" << endl;
        exit(1);
    }
    cout << "Loading config ... ";
    itim::config_branch cfg("../srch/config.json");
    cout << "done\nLoading global index ... ";
    itim::global_index gi(cfg.branch("index_buckets"));
    cout << "QUERY:" << endl;
    cout << argv[1] << endl;
    cout << "done\nParsing query ... ";
    itim::query_parser qp(cfg.branch("provinces"), gi);
    itim::query_tree qt;
    itim::special_search_params sp;
    qp.parse_params(argv[1], qt, sp);
    dumper d;
    d(qt);
    cout << "done\n";
    return 0;
}
