#ifndef MINUS_FILTER_H
#define MINUS_FILTER_H

#include <list>
#include <srch/itim_base.h>
#include <srch/itim_algo.h>

namespace itim {

    template<pass_type pt, depth d, class index_c>
    class minus_filter_base {
        protected:
        typedef typename index_c::template range<pt, d>::value minus_item_range;
        typedef std::list<minus_item_range> minus_item_ranges;
        typedef std::list<minus_item_ranges> minus_phrase_ranges;

        public:
        minus_filter_base() {}

        minus_filter_base(index_c &ind, const minus_phrase &mp)
        {
            for(minus_phrase::const_iterator i = mp.begin(); i != mp.end(); ++i) {
                mp_ranges.emplace_back(minus_item_ranges());
                for(minus_item::const_iterator j = i->begin(); j != i->end(); ++j)
                    mp_ranges.back().emplace_back(ind.template posting_iterator_range<pt, d>(*j));
            }
        }

        minus_filter_base &operator=(minus_filter_base &&other) {
            mp_ranges.swap(other.mp_ranges);
            return *this;
        }

        protected:
        bool is_exist_everywhere(
                const docid_t &docid, minus_item_ranges &ranges) {
            for(typename minus_item_ranges::iterator j = ranges.begin(); j != ranges.end(); ++j) {
                itim::lower_bound(*j, docid);
                if(j->empty() || j->front() != docid)
                    return false;
            }
            return true;
        }

        protected:
        minus_phrase_ranges mp_ranges;
    };

    template<pass_type pt, depth d, class index_c>
    class minus_filter {};

    template<depth d, class index_c>
    class minus_filter<draft, d, index_c> : public minus_filter_base<draft, d, index_c> {
        public:
        minus_filter() {}
        minus_filter(index_c &ind, const minus_phrase &mp) :
            minus_filter_base<draft, d, index_c>(ind, mp) {}

        bool operator()(const docid_t &docid) {
            for(typename minus_filter::minus_phrase_ranges::iterator i = this->mp_ranges.begin(); i != this->mp_ranges.end(); ++i)
                if(i->size() <= 1 && this->is_exist_everywhere(docid, *i))
                    return false;
            return true;
        }
    };

    template<depth d, class index_c>
    class minus_filter<full, d, index_c> : public minus_filter_base<full, d, index_c> {
        public:
        minus_filter() {}
        minus_filter(index_c &ind, const minus_phrase &mp) :
            minus_filter_base<full, d, index_c>(ind, mp) {}

        bool operator()(const docid_t &docid) {
            for(typename minus_filter::minus_phrase_ranges::iterator i = this->mp_ranges.begin(); i != this->mp_ranges.end(); ++i)
                if(is_exist_everywhere(docid, *i) &&
                        (i->size() <= 1 /*|| is_in_order(TODO)*/)) return false;
            return true;
        }
    };

}

#endif
