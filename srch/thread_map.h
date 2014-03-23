#include <boost/thread.hpp>
#include <srch/itim_base.h>

namespace itim {

template<class iterator, class lambda>
void thread_map(iterator first, iterator last, const lambda &func) {
    boost::thread_group *tg = new boost::thread_group;
    try {
        for(; first != last; ++first) {
            tg->create_thread([=] () { func(*first); });
        }
        tg->join_all();
        delete tg;
    } catch(...) {
        delete tg;
        throw;
    }
}

}
