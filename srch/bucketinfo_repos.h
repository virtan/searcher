#ifndef BUCKETINFO_REPOS_H
#define BUCKETINFO_REPOS_H

#include <fstream>
#include <string>
#include <algorithm>

#include <eger/profiler.h>

#include <srch/itim_base.h>
#include <srch/bucketinfo.pb.h>



namespace itim {

class bucketinfo_repository {
    public:
    bucketinfo_repository(const std::string& path) {
        profiler_start(bucketinfo_repository_loading);
        std::fstream in(path.c_str(), std::ios::in | std::ios::binary);
        if (!bi.ParseFromIstream(&in)) {
            throw exception("Failed to recognize the bucketinfo protobuf messsage")
                << where(__PRETTY_FUNCTION__) << file(path) << errno_code(errno);
        }
        profiler_dump_2(bucketinfo_repository_loading, this->get_global_corpus_size() << " items");
    }

    int64_t get_global_corpus_size() const {
        return bi.global_corpus_size();
    }

    private:
    proto::bucketinfo bi;
};

}

#endif

