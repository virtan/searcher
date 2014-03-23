#include <iostream>
#include <srch/itim_base.h>
#include <srch/globals.h>
#include <srch/query_parser.h>
#include <srch/searcher.h>

int main(int argc, char **argv) {
    try {
        itim::globals glb;
        glb.init(argc, argv);
        try {
            if(!glb.queries().size()) {
                log_critical("no queries given");
                return 1;
            }
            itim::global_index gi(glb.config().branch("index_buckets"));
            itim::query_parser qp(glb.config().branch("provinces"), gi);
            for(auto i = glb.queries().begin(); i != glb.queries().end(); ++i)  {
                itim::query_tree qt;
                itim::special_search_params sp;
                qp.parse_params(*i, qt, sp);
                itim::searcher s;
                itim::hit_list hl = s.search_web(gi, qt, sp);
                std::cout << "Hit list size: " << hl.size() << std::endl;
            }
        } catch(itim::exception &e) {
            log_critical("Exception: " << e.what() <<
                    ", file = " << (boost::get_error_info<itim::file>(e) ?
                        *boost::get_error_info<itim::file>(e) : "null") <<
                    ", where = " << (boost::get_error_info<itim::where>(e) ?
                        *boost::get_error_info<itim::where>(e) : "null"));
            return 1;
        }
    } catch(itim::exception &e) {
        std::cerr << "(no logger set): Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
