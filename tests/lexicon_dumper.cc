#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>
#include <algorithm>

#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/lexicon.h>
#include <srch/globals.h>
#include <srch/itim_algo.h>

using namespace std;
using namespace boost;
using namespace itim;

void tab(size_t i) {
    cout << string(i, ' ');
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;

    try {
    globals glb;
    glb.init(argc, argv, "");
    cout << "Dumping lexicons\n";
    auto lexicons_cfg = glb.config().branches("index_buckets.bucket");
    std::sort(lexicons_cfg.begin(), lexicons_cfg.end(),
            [&] (const config_branch &a, const config_branch &b) -> bool { return a.get<size_t>("id") < b.get<size_t>("id"); });
    for(auto i = lexicons_cfg.begin(); i != lexicons_cfg.end(); ++i) {
        tab(4);
        cout << "id = " << i->get<string>("id") << endl;
        mapped_region mr(i->branch("lexicon"));
        auto iter_range = itim::make_iterator_range(
                index_data_iterator<mapped_region>(mr),
                index_data_iterator<mapped_region>(mr, mr.size()));
        qatar_lexicon_codec<mapped_region> codec(iter_range);
        try {
            while(true) {
                lexicon_item_and_term item_term = codec.next();
                tab(8);
                cout << item_term << " # lc = \"" << itim::lowercase(item_term.term) << "\"" << endl;
            }
        } catch(out_of_range_exception &e) {
            // end of lexicon_index
        }
    }
    cout << "successfully finished" << endl;
    } catch(itim::exception &e) {
        cerr << e.what() <<
            ", file = " << (boost::get_error_info<itim::file>(e) ? 
                    *boost::get_error_info<itim::file>(e) : "null") <<
            ", where = " << (boost::get_error_info<itim::where>(e) ? 
                    *boost::get_error_info<itim::where>(e) : "null") <<
            endl;
    } catch(std::exception &e) {
        cerr << e.what() << endl;
    } catch(...) {
        cerr << "Some other exception" << endl;
    }

    return 0;
}
