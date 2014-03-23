#include <iostream>
#include <string.h>
#include <vector>

#include <boost/thread.hpp>

#include <srch/itim_base.h>
#include <srch/globals.h>
#include <srch/query_parser.h>
#include <srch/searcher.h>

using namespace std;
using namespace boost;

size_t done = 0;
vector<size_t> timings;
size_t global_df = 0;

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


void synchro_start() {
    unique_lock<mutex> start_lock(start);
    start_cond.wait(start_lock);
}

void update_results(size_t mcs) {
    unique_lock<mutex> finish_lock(finish);
    timings.push_back(mcs);
}

void atomic_add(size_t *object, size_t delta) {
    __sync_add_and_fetch(object, delta);
}

void test(size_t tests, itim::globals &glb, itim::global_index &gi, itim::query_parser &qp) {
    synchro_start();
    
    while(tests--) {

        for(auto i = glb.queries().begin(); i != glb.queries().end(); ++i)  {
            itim::query_tree qt;
            itim::special_search_params sp;
            qp.parse_params(*i, qt, sp);
            itim::searcher s;
            timer tmr;
            itim::hit_list hl = s.search_web(gi, qt, sp);
            update_results(tmr.current());
            atomic_add(&global_df, hl.size());
        }

    }

    unique_lock<mutex> finish_lock(finish);
    ++done;
    finish_cond.notify_one();
}

void wait_for_finish(size_t threads) {
    sleep(1);
    log_info("started");
    unique_lock<mutex> finish_lock(finish);
    start_cond.notify_all();
    while(done != threads)
        finish_cond.wait(finish_lock);
}

int main(int argc, char **argv) {
    namespace po = boost::program_options;
    try {
        itim::globals glb;
        string query_file;
        size_t threads;
        size_t count;
        glb.add_options()
            ("query_file", po::value<string>(&query_file),
             "list of queries")
            ("threads", po::value<size_t>(&threads)->default_value(1),
             "number of threads")
            ("count", po::value<size_t>(&count)->default_value(1),
             "number of passes in each thread")
            ;
        glb.init(argc, argv, "wi");
        try {
            if(!query_file.empty()) {
                ifstream is(query_file, ifstream::in);
                char buf[10240];
                size_t total = 0;
                while(!is.eof()) {
                    is.getline(buf, 10240);
                    if(!buf[0]) continue;
                    ++total;
                    glb.queries().push_back(buf);
                }
            }
            if(glb.queries().empty()) {
                log_critical("no queries given");
                return 1;
            }
            log_info("working with " << glb.queries().size() << " queries");
            itim::global_index gi(glb.config().branch("index_buckets"));
            itim::query_parser qp(glb.config().branch("provinces"), gi);

            list<thread> ts;

            for(size_t i = 0; i < threads; ++i)
                ts.emplace_back(test, count, std::ref(glb), std::ref(gi), std::ref(qp));

            wait_for_finish(threads);

            size_t min = size_t(0) - 1, max = 0, avg = 0;
            for(size_t i = 0; i < timings.size(); ++i) {
                if(min > timings[i]) min = timings[i];
                if(max < timings[i]) max = timings[i];
                avg += timings[i];
            }

            if(timings.size()) {
                avg /= timings.size();
                log_info("min = " << min << ", avg = " << avg << ", max = " << max);
            }


        } catch(itim::exception &e) {
            log_critical("Exception: " << e.what() <<
                    ", file = " << (*boost::get_error_info<itim::file>(e)) <<
                    ", where = " << (*boost::get_error_info<itim::where>(e)));
            return 1;
        }
    } catch(itim::exception &e) {
        std::cerr << "(no logger set): Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
