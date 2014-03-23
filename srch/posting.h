#ifndef POSTING_H
#define POSTING_H

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <unordered_map>
#include <srch/itim_base.h>
//#include <srch/itim_vector.h>

namespace itim {

struct posting {
    posting() : docid(no_docid) {}
    operator docid_t() { return docid; }
    operator const docid_t() const { return docid; }
    docid_t docid;
    bool operator< (const posting &other) const { return docid < other.docid; }
    bool operator> (const posting &other) const { return docid > other.docid; }
    bool operator<= (const posting &other) const { return docid <= other.docid; }
    bool operator>= (const posting &other) const { return docid >= other.docid; }
    bool operator== (const posting &other) const { return docid == other.docid; }
    bool operator!= (const posting &other) const { return docid != other.docid; }

    bool operator< (const docid_t &did) const { return docid < did; }
    bool operator> (const docid_t &did) const { return docid > did; }
    bool operator<= (const docid_t &did) const { return docid <= did; }
    bool operator>= (const docid_t &did) const { return docid >= did; }
    bool operator== (const docid_t &did) const { return docid == did; }
    bool operator!= (const docid_t &did) const { return docid != did; }
};

struct draft_posting : public posting
{
    draft_posting() : rank(no_rank) {}
    rank_t rank;
};

typedef std::vector<position_t> positions_t;
typedef std::unordered_map<alienid_t, positions_t> aliens_t;
//typedef fast_vector<position_t> positions_t;
//typedef fast_vector<positions_t> aliens_t;

class zone {
    public:
    zone() : zoneid(no_zoneid) {}
    zone(const zone &other) {
        operator=(other);
    }

    zone &operator=(const zone &other) {
        if(other.positions_) positions_.reset(new positions_t(*other.positions_));
            else positions_.reset(NULL);
        if(other.aliens_) aliens_.reset(new aliens_t(*other.aliens_));
            else aliens_.reset(NULL);
        zoneid = other.zoneid;
        return *this;
    }

    zoneid_t zoneid;
    positions_t &positions() {
        if(!positions_) {
            // not thread safe
            positions_.reset(new positions_t());
        }
        return *positions_;
    }
    aliens_t &aliens() {
        if(!aliens_) {
            // not thread safe
            aliens_.reset(new aliens_t());
        }
        return *aliens_;
    }

    private:
    boost::scoped_ptr<positions_t> positions_;
    boost::scoped_ptr<aliens_t> aliens_;
};

typedef std::vector<zone> zones_t;
//typedef fast_vector<zone> zones_t;

struct full_posting : public posting
{
    full_posting() {}
    zones_t zones;
};

}

#endif
