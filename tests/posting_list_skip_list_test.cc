#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>
#include <algorithm>

#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/index.h>
#include <srch/globals.h>

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
    size_t offset;
    size_t index_id;
    glb.add_options()
        ("offset", po::value<size_t>(&offset)->default_value(0),
         "index offset")
        ("index-id", po::value<size_t>(&index_id),
         "index id")
        ;
    glb.init(argc, argv, "");
    auto buckets = glb.config().branches("index_buckets.bucket");
    auto index_id_bucket = std::find_if(buckets.begin(), buckets.end(),
            [&] (const config_branch &a) -> bool { return a.get<size_t>("id") == index_id; });
    if(index_id_bucket == buckets.end()) {
        log_critical("index with id " << index_id << " not found");
        return 1;
    }
    cout << "Dumping index " << index_id << " from " << offset << "\n";
    mapped_region mreg(index_id_bucket->branch("index"));
    lexicon_item li(0, 0, boost::assign::list_of(offset)(0)(0)(0)(0)(0));
    block blk = b1k;
    auto begin = posting_iterator<qatar_draft_codec<mapped_region> >(boost::make_tuple(boost::cref(li), boost::cref(blk), boost::ref(mreg), (0 - 1)));
    auto end = posting_iterator<qatar_draft_codec<mapped_region> >();

    for(size_t p = 0; begin != end; ++begin, ++p) {
        if(p % 3 == 0) {
            if(p) cout << "\n";
            tab(8);
        } else cout << "\t";
        auto begin_skiplist = posting_iterator<qatar_draft_codec<mapped_region> >(boost::make_tuple(boost::cref(li), boost::cref(blk), boost::ref(mreg), (0 - 1)));
        begin_skiplist.lower_bound(begin->docid);
        cout << (p + 1) << ": ";
        cout << *begin << (*begin == *begin_skiplist ? " == " : " != ") << *begin_skiplist;
    }
    cout << "\nSuccessfully finished" << endl;
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
