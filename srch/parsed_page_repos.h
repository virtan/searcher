#ifndef PARSED_PAGE_REPOS_H
#define PARSED_PAGE_REPOS_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/scoped_ptr.hpp>

#include <eger/profiler.h>

#include <srch/mapped_region.h>
#include <srch/itim_base.h>
#include <srch/parsed_page.pb.h>
#include <srch/parsed_page.h>
#include <srch/boost/optional.hpp>



namespace itim {

class parsed_page_repository {
    public:
    parsed_page_repository(const std::string &index_path, const std::string &data_path): pp_region(new mapped_region(data_path)) {
        profiler_start(parsed_page_repository_loading);
        std::fstream index_file(index_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!index_file.is_open()) {
            throw exception("Failed to open the parsed page index")
                << where(__PRETTY_FUNCTION__) << file(index_path) << errno_code(errno);
        }

        index_size = index_file.tellg() / sizeof(docid_t);
        index.resize(index_size);
        index_file.seekg(0, std::ios::beg);
        index_file.read((char *)&index[0], index_size * sizeof(docid_t));
        profiler_dump(parsed_page_repository_loading, this->index_size << " items");
    }

    const boost::optional<parsed_page> get(docid_t docid) const {
        if (docid >= index.size() ) {
            return boost::optional<parsed_page>();
        }
        size_t pp_length;
        pp_length = ( docid + 1 < index.size() ) ?
            ( index[docid+1] - index[docid]) :
            ( pp_region->get_size() - index[docid] );
        char * pp_address = (char *) pp_region->get_address() + index[docid];
        proto::parsed_page *pp_message = new proto::parsed_page;
        if (!pp_message->ParseFromArray(pp_address, pp_length)) {
            throw exception("Failed to parse the parsed_page protobuf message")
                << where(__PRETTY_FUNCTION__);
        };
        return boost::optional<parsed_page>(parsed_page(pp_message));
    }

    private:
    size_t index_size;
    std::vector<docid_t> index;
    boost::scoped_ptr<mapped_region> pp_region;

};

}


#endif
