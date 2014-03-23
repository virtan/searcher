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
#include <srch/index.h>

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
    size_t amount;
    size_t multip;
    string output;
    glb.add_options()
        ("amount", po::value<size_t>(&amount)->default_value(100), "amount of posting lists")
        ("multip", po::value<size_t>(&multip)->default_value(3200), "multiplier for posting list length")
        ("output", po::value<string>(&output)->default_value("dump"), "filename to dump to");
    glb.init(argc, argv, "");
    cout << "Seeking for " << amount << " " << multip << "-multiple posting_lists...\n";
    auto buckets = glb.config().branches("index_buckets.bucket");
    std::sort(buckets.begin(), buckets.end(),
            [&] (const config_branch &a, const config_branch &b) -> bool { return a.get<size_t>("id") < b.get<size_t>("id"); });
    size_t output_width = 100;
    std::map<size_t, size_t> distribution;
    size_t total_found = 0;
    ofstream outputf(output);
    for(auto i = buckets.begin(); total_found < amount && i != buckets.end(); ++i) {
        // cout << "id = " << i->get<string>("id") << endl;
        itim::index<> indx(*i);
        try {
            for(auto j = indx.lexicon_ref().begin(); j != indx.lexicon_ref().end(); ++j) {
                if(j->second.df >= multip) {
                    if(total_found % output_width == 0) {
                        if(total_found) cout << "\n";
                        tab(4);
                    }
                    cout << "*"; cout.flush();
                    size_t posting_length = j->second.df / multip * multip;
                    ++distribution[posting_length];

                    // dumper
                    outputf << j->first << endl;
                    auto irange = indx.posting_iterator_range<draft, all_blocks>(j->first);
                    while(posting_length--) {
                        outputf << ' ' << irange.front().docid.local_docid;
                        irange.pop_front();
                    }
                    outputf << endl;

                    if(++total_found >= amount) break;
                }
            }
        } catch(out_of_range_exception &e) {
            // end of lexicon_index
        }
    }
    cout << endl << "dumped " << total_found << " posting lists\ndistribution: " << endl;
    for(auto i = distribution.begin(); i != distribution.end(); ++i) {
        tab(4);
        cout << i->first << "\t" << i->second << endl;
    }
    cout << "done" << endl;
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
