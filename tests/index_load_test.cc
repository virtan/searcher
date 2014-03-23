#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>

#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/index.h>

using namespace std;
using namespace boost;

int done = 0;
vector<size_t> timings;
vector<string> words;
size_t global_df = 0;
int depth = 1;
int ptype = 1;
size_t cycle_count = 0;

mutex start;
condition_variable start_cond;

mutex finish;
condition_variable finish_cond;

class timer {
    public:
        timer() { start(); }
        void start() { gettimeofday(&start_time, NULL); }
        size_t current() {
            struct timeval now;
            gettimeofday(&now, NULL);
            size_t diff = (((int64_t) now.tv_sec) * 1000000) + now.tv_usec - (((int64_t) start_time.tv_sec) * 1000000) - start_time.tv_usec;
            return diff;
        }

    private:
        struct timeval start_time;
};

template<itim::pass_type pt, itim::depth d>
size_t iteration(itim::index<> *ind, const itim::term_type &t) {
    auto range = ind->posting_iterator_range<pt, d>(t);
    size_t rv = 0;
    //size_t internal_counter = 0;
    for(; !range.empty(); range.pop_front()) {
        //if(internal_counter++) cout << ",";
        rv += (uint32_t) range.front().docid.local_docid;
        //cout << (uint32_t) range.front().docid;
        //__sync_add_and_fetch(&cycle_count, 1);
    }
    __sync_add_and_fetch(&global_df, rv);
    //cout << endl;
    return rv;
}

void test(int tests, itim::index<> *ind) {
    {
        unique_lock<mutex> start_lock(start);
        start_cond.wait(start_lock);
    }
    
    while(tests--) {
        timer tmr;
        for(size_t i = 0; i < words.size(); ++i) {
            switch(depth*10 + ptype) {
                case 11:
                    iteration<itim::draft, itim::first_block>(ind, words[i]);
                    break;
                case 21:
                    iteration<itim::draft, itim::top_blocks>(ind, words[i]);
                    break;
                case 31:
                    iteration<itim::draft, itim::all_blocks>(ind, words[i]);
                    break;
                case 12:
                    iteration<itim::full, itim::first_block>(ind, words[i]);
                    break;
                case 22:
                    iteration<itim::full, itim::top_blocks>(ind, words[i]);
                    break;
                case 32:
                    iteration<itim::full, itim::all_blocks>(ind, words[i]);
                    break;
                default:
                    break;
            }
        }
        size_t mcs = tmr.current();
        unique_lock<mutex> finish_lock(finish);
        timings.push_back(mcs);
    }

    unique_lock<mutex> finish_lock(finish);
    ++done;
    finish_cond.notify_one();
}

void help(const char *p) {
        cerr << "Usage: " << p << " <number_of_tests> <number_of_threads> <words_path> [<depth = 1'|2|3>] [<type = 1'(draft)|2(full)>]" << endl;
        exit(1);
}

int main(int argc, char **argv) {
    if(argc < 4) help(argv[0]);

    int tests = atoi(argv[1]);
    int threads = atoi(argv[2]);
    const char *words_path = argv[3];
    if(argc >= 5) {
        depth = atoi(argv[4]);
        if(depth < 1 || depth > 3) help(argv[0]);
    }

    if(argc >= 6) {
        ptype = atoi(argv[5]);
        if(ptype < 1 || ptype > 2) help(argv[0]);
    }


    timings.reserve(tests);
    tests /= threads;

    itim::config_branch cfg("../srch/config.json");
    cout << "Loading index ...";
    cout.flush();
    itim::config_branch one_bucket = cfg.branches("index_buckets.bucket")[0];
    itim::index<> ind(one_bucket);
    cout << " done\n";

    {
        cout << "Checking test words ...\n";
        ifstream is(words_path, ifstream::in);
        char buf[256];
        size_t total = 0;
        while(!is.eof()) {
            is.getline(buf, 256);
            if(!buf[0]) continue;
            ++total;
            auto iter = ind.lexicon_ref().find(buf);
            if(iter == ind.lexicon_ref().end()) {
                cout << "\t" << buf << " doesn't exist\n";
            } else {
                words.push_back(buf);
            }
        }
        cout << (total - words.size()) << " (" << ((total - words.size()) * 100 / total) <<
            "%) words doesn't exist, testing " << words.size() << " words\n";
    }

    list<thread> ts;

    for(int i = 0; i < threads; ++i)
        ts.emplace_back(test, tests, &ind);

    cout << "Pause before testing ...";
    cout.flush();
    sleep(1);
    cout << " done\n";

    cout << endl;

    unique_lock<mutex> finish_lock(finish);

    start_cond.notify_all();

    while(done != threads)
        finish_cond.wait(finish_lock);

    size_t min = size_t(0) - 1, max = 0, avg = 0;
    for(size_t i = 0; i < timings.size(); ++i) {
        cout << i << " " << timings[i] << endl;
        if(min > timings[i]) min = timings[i];
        if(max < timings[i]) max = timings[i];
        avg += timings[i];
    }

    if(timings.size()) {
        avg /= timings.size();
        cout << "min = " << min << ", avg = " << avg << ", max = " << max << endl;
    }

    //cout << "Cycle count: " << cycle_count << endl;

    cout << "Summ of docids: " << global_df << endl;

    // prevent destruction
    //_exit((int) global_df);
    return 0;
}
