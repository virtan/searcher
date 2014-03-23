#ifndef WEB_PAGE_INFO_REPOS_H
#define WEB_PAGE_INFO_REPOS_H

#include <stdint.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <eger/profiler.h>

#include <srch/itim_types.h>
#include <srch/config_branch.h>
#include <srch/wpi.pb.h>
#include <srch/itim_exception.h>
#include <srch/web_page_info.h>

#pragma pack(push, 1)
namespace itim {

class web_page_info_repository {
public:
    web_page_info_repository(const config_branch &wpi_cfg) {
        profiler_start(web_page_info_repository_loading);
        load(wpi_cfg.get<string>("file"));
        profiler_dump_2(web_page_info_repository_loading, wpis.size() << " items");
    }

    web_page_info get(docid_t docid) const {
        if ( docid.local_docid >= wpis.size() ) {
            return web_page_info();
            // return NULL;
        }
        return wpis[docid.local_docid];
    }

    size_t get_corpus_size() const {
        return corpus_size;
    }

private:
    uint32_t corpus_size;
    std::vector<position_t> positions;
    std::vector<web_page_info> wpis;

private:
    void load(const std::string& path) {
        std::fstream fstream(path.c_str(), std::ios::in | std::ios::binary);
        if (!fstream.is_open()) {
            throw exception("Failed to find the web_page_info file")
                << where(__PRETTY_FUNCTION__) << file(path) << errno_code(errno);
        }

        // read the whole file into a buffer
        fstream.seekg (0, std::ios::end);
        size_t file_length = fstream.tellg();
        fstream.seekg (0, std::ios::beg);
        std::vector<char> file_buffer(file_length);
        fstream.read(&file_buffer[0], file_length);
        fstream.close();

        // stream semantics for the buffer
        std::stringstream file_buf_stream;
        file_buf_stream.rdbuf()->pubsetbuf(&file_buffer[0],file_length);
        file_buf_stream.read((char*)&corpus_size, sizeof(uint32_t));

        // read the position table
        positions.resize(corpus_size);
        file_buf_stream.read((char*)&positions[0], corpus_size*sizeof(position_t));

        // reserve the vector
        wpis.reserve(corpus_size);

        // parse the messages straight from the buffer
        for(size_t i = 0; i < corpus_size; i++) {
            position_t start_pos =  positions[i];
            size_t msg_length;
            if (corpus_size == i+1) {
                msg_length = file_length - positions[i];
            } else {
                msg_length = positions[i+1] - positions[i];
            }
            proto::web_page_info* wpi_msg = new proto::web_page_info();
            if(!wpi_msg->ParseFromArray(&file_buffer[start_pos], msg_length)) {
                throw exception("Failed to parse the web_page_info protobuf message")
                    << where(__PRETTY_FUNCTION__) << file(path);
            };
            wpis.emplace_back(wpi_msg);
        }
    }

};

}
#pragma pack(pop)

#endif
