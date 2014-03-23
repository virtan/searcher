#include <string.h>
#include <iostream>
#include <vector>
#include <string>
#include <boost/thread.hpp>

#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/lexicon.h>

using namespace std;
using namespace boost;

int done = 0;
vector<size_t> timings;
vector<string> words;
uint32_t global_df = 0;

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

void test(int tests, itim::lexicon<> *lex) {
    {
        unique_lock<mutex> start_lock(start);
        start_cond.wait(start_lock);
    }
    
    while(tests--) {
        timer tmr;
        for(size_t i = 0; i < words.size(); ++i)
            global_df += (*lex)[words[i]].df;
        size_t mcs = tmr.current();
        unique_lock<mutex> finish_lock(finish);
        timings.push_back(mcs);
    }

    unique_lock<mutex> finish_lock(finish);
    ++done;
    finish_cond.notify_one();
}

int main(int argc, char **argv) {
    if(argc < 4) {
        cerr << "Usage: " << argv[0] << " <number_of_tests> <number_of_threads> <words_path>" << endl;
        exit(1);
    }

    int tests = atoi(argv[1]);
    int threads = atoi(argv[2]);
    const char *words_path = argv[3];

    timings.reserve(tests);
    tests /= threads;

    itim::config_branch cfg("../src/config.json");
    cout << "Loading lexicon ...";
    cout.flush();
    itim::lexicon<> lex(cfg.branch("data.lexicon"));
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
            auto iter = lex.find(buf);
            if(iter == lex.end()) {
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
        ts.emplace_back(test, tests, &lex);

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

    // prevent destruction
    _exit((int) global_df);
    return 0;
}
