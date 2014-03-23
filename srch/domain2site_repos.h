#ifndef DOMAIN2SITE_REPOS_H
#define DOMAIN2SITE_REPOS_H

#include <fstream>
#include <string>
#include <algorithm>
#include <boost/optional.hpp>
#include <unordered_map>
#include <google/protobuf/repeated_field.h>
#include <eger/profiler.h>
#include <srch/itim_base.h>
#include <srch/config_branch.h>
#include <srch/domain2site.pb.h>


#pragma pack(push, 1)
namespace itim {

class domain2site_repository: private std::unordered_map<string, siteid_t>  {
    public:
    domain2site_repository(const config_branch &d2s_cfg) {
        profiler_start(domain2site_repository_loading);
        load(d2s_cfg.get<string>("file"));
        profiler_dump_2(domain2site_repository_loading, this->size() << " items");
    }

    siteid_t get(string key) {
        if ( this->count(key) == 1 ) {
            return this->at(key);
        }
        return no_siteid;
    }

    private:
    void load(const string& path) {
        proto::domain2site d2s;
        std::fstream in(path.c_str(), std::ios::in | std::ios::binary);
        if (!d2s.ParseFromIstream(&in)) {
            throw exception("Failed to recognize the domain2site protobuf messsage")
                << where(__PRETTY_FUNCTION__) << file(path) << errno_code(errno);
        }
        if (d2s.domains().size() != d2s.siteids().size()) {
            throw exception("The domain2site protobuf messsage is malformed: domain and siteid arrays differ")
                << where(__PRETTY_FUNCTION__);
        }
        for (size_t i = 0; i != (size_t)d2s.domains().size(); ++i) {
            this->insert(std::pair<string, siteid_t>(d2s.domains(i), d2s.siteids(i)));
        }
    }
};

}
#pragma pack(pop)

#endif
