#ifndef COMPOSITE_POSTING_ITERATOR_H
#define COMPOSITE_POSTING_ITERATOR_H

#include <list>
#include <boost/iterator/iterator_facade.hpp>
#include <srch/itim_base.h>
#include <srch/itim_algo.h>
#include <srch/itim_iterator_range.h>
#include <srch/posting.h>
#include <srch/index.h>

namespace itim {

    template<pass_type pt, depth d>
    struct composite_posting_iterator_return_type {};

    template<depth d>
    struct composite_posting_iterator_return_type<draft, d> {
        typedef draft_posting value_type;
    };

    template<depth d>
    struct composite_posting_iterator_return_type<full, d> {
        typedef full_posting value_type;
    };

    template<class composite, pass_type pt, depth d>
    class composite_posting_iterator :
        public boost::iterator_facade<
        composite_posting_iterator<composite, pt, d>,
        const typename composite_posting_iterator_return_type<pt, d>::value_type &,
        boost::forward_traversal_tag>
    {
        public:
        typedef typename composite_posting_iterator_return_type<pt, d>::value_type value_type;

        private:
        typedef typename index<>::range<pt, d>::value index_range_type;
        struct composite_index_range : public index_range_type {
            template<typename... Args>
            composite_index_range(const bucketid_t &_bucketid, const size_t _id,
                    Args&&... args) :
                index_range_type(args...),
                bucketid(_bucketid),
                id(_id)
            {}
            bucketid_t bucketid;
            size_t id;
        };
        typedef std::list<composite_index_range> index_ranges;
        enum actualize_from { load, end };

        public:
        composite_posting_iterator() :
            prev_range_scanned(0),
            end_flag(true)
        {}

        composite_posting_iterator(const term_type &term, typename composite::composite_index &_source) :
            prev_range_scanned(0),
            end_flag(false)
        {
            bucketid_t bucketid_counter = 0;
            for (auto i = _source.begin(); i != _source.end(); i++) {
                ranges.emplace_back(bucketid_counter, i->id, i->template posting_iterator_range<pt,d>(term));
                log_debug_mare("composite_posting_iterator[" << i->id << "].constructor(\"" <<
                        term << "\") = " << (this->ranges.back().empty() ? "empty" : "not empty"));
                if(ranges.back().empty()) ranges.pop_back();
                bucketid_counter++;
            }
            actualize_current_posting(ranges.empty() ? end : load);
        }

    private:
        friend class boost::iterator_core_access;

        const value_type &dereference() const {
            return current_posting;
        }

        void increment() {
            if(end_flag) return;
            ranges.front().pop_front();
            if(ranges.front().empty()) {
                next_range();
                if(ranges.empty()) {
                    actualize_current_posting(end);
                    return;
                }
            }
            actualize_current_posting(load);
        }

        bool equal(composite_posting_iterator const &other) const {
            if(end_flag || other.end_flag)
                return end_flag == other.end_flag;
            return current_posting.docid == other.current_posting.docid; // may be insufficient
        }

        void next_range() {
            prev_range_scanned += ranges.front().begin().scanned();
            auto __attribute__((unused)) prev_range_id = ranges.front().id;
            ranges.pop_front();
            log_debug_mare("composite_posting_iterator[" << prev_range_id <<
                    "].next_range() = " << (this->ranges.empty() ? string("none") :
                        boost::lexical_cast<string>(this->ranges.front().id))
                    << " (total_scanned = " << this->prev_range_scanned << ")");
        }

        inline void actualize_current_posting(actualize_from af) {
            switch(af) {
                case load:
                    current_posting = ranges.front().front();
                    current_posting.docid.bucketid = ranges.front().bucketid;
                    break;
                case end:
                    end_flag = true;
                    current_posting = value_type();
                    break;
            }
        }

    public:
        void lower_bound(const docid_t& docid) {
            if(end_flag) return;
            for(;!ranges.empty() && ranges.front().bucketid < docid.bucketid; next_range());
            if(ranges.empty()) { actualize_current_posting(end); return; }
            if(ranges.front().bucketid == docid.bucketid) {
                itim::lower_bound(ranges.front(), docid_t(docid.local_docid, no_bucketid));
                if(ranges.front().empty()) next_range();
            }
            actualize_current_posting(ranges.empty() ? end : load);
        }

        size_t scanned() const {
            return prev_range_scanned + (ranges.empty() ? 0 :
                    ranges.front().empty() ? 0 : ranges.front().begin().scanned());
        }

        size_t size() const {
            size_t sz = 0;
            for(typename index_ranges::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
                sz += i->begin().size();
            return sz;
        }

    private:
        index_ranges ranges;
        size_t prev_range_scanned;
        value_type current_posting;
        bool end_flag;
    };

}

#endif
