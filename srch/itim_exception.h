#ifndef ITIM_EXCEPTION_H
#define ITIM_EXCEPTION_H

#include <stdexcept>
#include <itim_string.h>
#include <boost/exception/all.hpp>

namespace itim {

typedef boost::error_info<struct tag_where, const char *> where;
typedef boost::error_info<struct tag_file, string> file;
typedef boost::error_info<struct tag_errno_code, int> errno_code;
typedef boost::error_info<struct tag_reason, string> reason;

class exception : virtual public std::exception, virtual public boost::exception {
    public:
    exception(const std::string &_reason) { *this << reason(_reason); }
    virtual const char *what() const throw() {
        const std::string* reason_ = boost::get_error_info<reason>(*this);
        return reason_ ? reason_->c_str() : "No itim::exception reason provided";
    }
};

class out_of_range_exception : virtual public exception {
    public:
        out_of_range_exception(const std::string &_reason) : exception(_reason) {}
};

}

#endif
