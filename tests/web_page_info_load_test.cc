#include <srch/globals.h>
#include <srch/config_branch.h>
#include <srch/web_page_info_repos.h>

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

int main(int argc, char** argv) {
    itim::globals glb;
    glb.init(argc, argv);
    log_debug("Starting the web page info load test...");
    itim::config_branch wpi_config = glb.config().branch("index_buckets.bucket.webpageinfo");

    timer tmr;
    for (size_t i = 0; i < 1; ++i) {
        itim::web_page_info_repository wpi_repo(wpi_config);
    }
    log_debug("Done in " << tmr.current() << " mcs");
    return 0;
}
