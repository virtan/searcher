#ifndef PARSED_PAGE_H
#define PARSED_PAGE_H

#include <srch/parsed_page.pb.h>
#include <boost/shared_ptr.hpp>


namespace itim {

class parsed_page {
    public:
    parsed_page() {}
    parsed_page(proto::parsed_page *pp_message_): pp_message(pp_message_) {}

    private:
    boost::shared_ptr<proto::parsed_page> pp_message;
};

}

#endif
