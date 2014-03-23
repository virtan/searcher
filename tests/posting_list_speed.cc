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

const size_t threads = 2;
const size_t work_sec = 100;

mutex start;
condition_variable start_cond;

mutex finish;
condition_variable finish_cond;

size_t counters[threads];

size_t done = 0;


template<class T>
void test(size_t id, T &begin) {
    {
        unique_lock<mutex> start_lock(start);
        start_cond.wait(start_lock);
    }
    
    T end = posting_iterator<qatar_draft_codec<mapped_region> >();
    counters[id] = 0;
    eger::timer t;
    for(size_t i = 0;; ++i) {
	if(!(i % 1000))
            if(t.current() > work_sec * 1000000) { break; }
        T ibegin = begin;
        while(ibegin != end) { ++ibegin; ++counters[id]; }
    }

    unique_lock<mutex> finish_lock(finish);
    ++done;
    finish_cond.notify_one();
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
    cout << "Calculating speed of reading index " << index_id << " from " << offset << "\n";
    mapped_region mreg(index_id_bucket->branch("index"));
    lexicon_item li(0, 0, boost::assign::list_of(offset)(0)(0)(0)(0)(0));
    block blk = b1k;
    auto begin = posting_iterator<qatar_draft_codec<mapped_region> >(boost::make_tuple(boost::cref(li), boost::cref(blk), boost::ref(mreg), (0 - 1)));
    //auto end = posting_iterator<qatar_draft_codec<mapped_region> >();

    list<thread> ts;
    for(size_t i = 0; i < threads; ++i)
        ts.emplace_back(test<decltype(begin)>, i, begin);
    sleep(1);

    eger::timer total_time;
    unique_lock<mutex> finish_lock(finish);
    start_cond.notify_all();
    while(done != threads)
        finish_cond.wait(finish_lock);
    cout << "Total time passed: " << ((double) total_time.current() / 1000000) << " sec" << endl;

    size_t counter = 0;
    for(size_t i = 0; i < threads; ++i) {
        cout << "    " << (counters[i] / 1000000);
        counter += counters[i];
    }
    cout << "\nSuccessfully finished: " << (counter / 1000000) << " mln iterations"  << endl;
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

