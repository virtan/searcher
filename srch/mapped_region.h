#ifndef MAPPED_REGION_H
#define MAPPED_REGION_H

#include <string>
#include <sys/mman.h>
#ifdef MAP_POPULATE
#define mmap(A1, A2, A3, A4, A5, A6) mmap(A1, A2, A3, ((A4) | MAP_POPULATE | MAP_NORESERVE), A5, A6)
#endif
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <eger/profiler.h>
#include <srch/config_branch.h>

namespace itim {
    using boost::interprocess::read_only;
    using boost::interprocess::read_private;
    using boost::interprocess::read_write;
    using boost::interprocess::copy_on_write;

    class mapped_region :
            public boost::interprocess::file_mapping,
            public boost::interprocess::mapped_region
            
    {
        public:
        mapped_region(const config_branch &cfg, size_t _id = 0) try :
            boost::interprocess::file_mapping(
                    cfg.get<string>("file").c_str(), read_only), 
            boost::interprocess::mapped_region(*this, read_only),
            summ(0),
            id(_id)
        {
            profiler_start(mapped_region_initialization);
            if(cfg.check_boolean("mlock"))
                assert(!mlock(get_address(), size()));
            if(cfg.check_boolean("pre_cache"))
                for(uint8_t *p = (uint8_t*) get_address(), *end = (uint8_t*) get_address() + size();
                        p < end; ++p) summ += *p;
            profiler_dump_2(mapped_region_initialization, (size()/1024/1024) << " megabytes");

        } catch(boost::interprocess::interprocess_exception &e) {
            throw exception(e.what()) <<
                where(__PRETTY_FUNCTION__) <<
                errno_code(errno) <<
                file(cfg.get<string>("file"));
        }

        mapped_region(const std::string& path) :
            boost::interprocess::file_mapping(
                path.c_str(), read_only),
            boost::interprocess::mapped_region(*this, read_only)
        {}

        size_t size() const { return get_size(); }

        private:
        size_t summ;

        public:
        size_t id;
    };

    class be_mapped_region : public mapped_region
    {
        public:
        be_mapped_region(const config_branch &cfg) : mapped_region(cfg) {}
    };
}

#endif
