#ifndef PHRASE_ITERATOR_H
#define PHRASE_ITERATOR_H

#include <list>
#include <vector>
#include <queue>
#include <boost/iterator/iterator_facade.hpp>
#include <eger/timer.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/itim_algo.h>

namespace itim {

    enum phrase_iterator_t {
        synonym_isec_iterator_e,     // 0
        synonym_merge_iterator_e,    // 1
        plus_group_iterator_e,       // 2
        plus_phrase_iterator_e,      // 3
        quote_group_iterator_e,      // 4
        quote_phrase_iterator_e,     // 5
        main_synonym_iterator_e,     // 6
        main_group_iterator_e,       // 7
        other_group_iterator_e,      // 8
        fa_main_group_iterator_e,    // 9
        fa_other_group_iterator_e    // 10 
    };

    std::ostream &operator<<(std::ostream &s, phrase_iterator_t pi) {
        switch(pi) {
            case synonym_isec_iterator_e: s << "synonym_isec_iterator_e"; break;
            case synonym_merge_iterator_e: s << "synonym_merge_iterator_e"; break;
            case plus_group_iterator_e: s << "plus_group_iterator_e"; break;
            case plus_phrase_iterator_e: s << "plus_phrase_iterator_e"; break;
            case quote_group_iterator_e: s << "quote_group_iterator_e"; break;
            case quote_phrase_iterator_e: s << "quote_phrase_iterator_e"; break;
            case main_synonym_iterator_e: s << "main_synonym_iterator_e"; break;
            case main_group_iterator_e: s << "main_group_iterator_e"; break;
            case other_group_iterator_e: s << "other_group_iterator_"; break;
            case fa_main_group_iterator_e: s << "fa_main_group_iterator_e"; break;
            case fa_other_group_iterator_e: s << "fa_other_group_iterator_"; break;
        }
        return s;
    }


    template<phrase_iterator_t pi_type, pass_type pt, depth d, class index_c>
    struct phrase_iterator_types { /* should define: sub_iterator, pi_range, source_type */ };

    template<phrase_iterator_t pi_type, pass_type pt, depth d, class index_c>
    struct phrase_iterator_extra {};

    template<phrase_iterator_t pi_type, pass_type pt, depth d, class index_c>
    class phrase_iterator :
        public boost::iterator_facade<
            phrase_iterator<pi_type, pt, d, index_c>,
            const docid_t&,
            boost::forward_traversal_tag>
    {
        protected:
        typedef typename phrase_iterator_types<pi_type, pt, d, index_c>::sub_iterator sub_iterator;
        typedef typename phrase_iterator_types<pi_type, pt, d, index_c>::pi_range pi_range;
        typedef std::list<pi_range> pi_ranges;
        typedef typename phrase_iterator_types<pi_type, pt, d, index_c>::source_type source_type;

        typedef std::pair<docid_t, typename pi_ranges::iterator> merge_pair;
            struct merge_pair_greater { bool operator()(const merge_pair &a, const merge_pair &b)
                { return a.first > b.first; } };
        typedef std::priority_queue<merge_pair, std::vector<merge_pair>, merge_pair_greater> merge_pqueue_t;
        struct merge_aux_t { merge_pqueue_t pq; std::vector<typename pi_ranges::iterator> pv; };
        struct pi_ranges_it_hash {
            size_t operator()(const typename pi_ranges::iterator &i) const {
                return (size_t) (&(*i));
            }
        };
        typedef std::unordered_map<typename pi_ranges::iterator, typename source_type::const_iterator,
                pi_ranges_it_hash> corresponding_source_locator_t;
        typedef std::multimap<size_t, typename pi_ranges::iterator> intersection_map_t;

        enum iterator_logic_type { merge_type, intersection_type };

        public:
        phrase_iterator() :
            current_docid(no_docid),
            next_docid(no_docid),
            end_flag(true),
            appearance_(0),
            score_cache_docid(no_docid)
        {}

        phrase_iterator(const source_type &_source, eger::timer _timeout) :
            current_docid(no_docid),
            next_docid(no_docid),
            end_flag(false),
            source(&_source),
            appearance_(0),
            score_cache_docid(no_docid),
            timeout(_timeout)
        {}

        phrase_iterator(phrase_iterator &&other) {
            *this = std::move(other);
        }

        phrase_iterator &operator=(phrase_iterator &&other) {

            boost::iterator_facade<
                phrase_iterator<pi_type, pt, d, index_c>,
                const docid_t&,
                boost::forward_traversal_tag>::operator=(other);
            current_docid = std::move(other.current_docid);
            next_docid = std::move(other.next_docid);
            end_flag = std::move(other.end_flag);
            ranges.swap(other.ranges);
            source = std::move(other.source);
            appearance_ = std::move(other.appearance_);
            score_cache_docid = std::move(other.score_cache_docid);
            score_cache_value = std::move(other.score_cache_value);
            timeout = std::move(other.timeout);
            merge_aux.pq.swap(other.merge_aux.pq);
            merge_aux.pv.swap(other.merge_aux.pv);
            corresponding_source_locator.swap(other.corresponding_source_locator);
            intersection_map.swap(other.intersection_map);
            extra = std::move(other.extra);
            return *this;
        }

        phrase_iterator(const phrase_iterator&) { assert(false); }
        phrase_iterator &operator=(const phrase_iterator&) { assert(false); return &this; }

        protected:
        void initialize(index_c &ind) {
            this->fill_ranges(ind);
            this->extra_constructor(ind, *source);
            log_debug_mare(pi_type << ".constructor(" <<
                    this->source->size() << ") = " << this->ranges.size());
            if(ranges.empty()) {
                next_docid = current_docid = no_docid;
                end_flag = true;
            }
            this->increment();
        }

        protected:
        virtual void fill_ranges(index_c &ind) {
            for(typename source_type::const_iterator i = source->begin();
                    i != source->end(); ++i)
                ranges.emplace_back(this->source_item_to_range(ind, *i));
        }

        virtual void extra_constructor(index_c&, const source_type&) {}

        virtual pi_range source_item_to_range(index_c &, const typename source_type::value_type &) {
            abort();
            return pi_range();
        }

        protected:
        friend class boost::iterator_core_access;

        const docid_t &dereference() const { return current_docid; }

        virtual void increment() {
            if(end_flag) return;
            this->lower_bound(current_docid == no_docid ? docid_t(0, 0) : current_docid + 1);
        }

        bool equal(phrase_iterator const &other) const {
            if(end_flag || other.end_flag)
                return end_flag == other.end_flag;
            return current_docid == other.current_docid; // may be insufficient
        }

        public:
        virtual void lower_bound(docid_t min_docid) = 0;

        protected:
        void lower_bound_intersection(docid_t min_docid, size_t rc_size = size_t(0) - 1) {
            if(end_flag) return;
            if(rc_size == size_t(0) - 1) rc_size = ranges.size();
            bool got_timeout = false; //timeout.deadline();
            if(got_timeout || !intersection(ranges, current_docid, min_docid, intersection_map, rc_size)) {
                next_docid = current_docid = no_docid;
                appearance_ = 0;
                end_flag = true;
            }
            appearance_ = ranges.size();
            if(got_timeout) {
                log_debug_hard(pi_type << ".lower_bound(" << min_docid << ")  = timeout ("
                        << this->timeout.deadline_description() << ")");
            } else {
                log_debug_mare(pi_type << ".lower_bound(" << min_docid << ") = " <<
                        this->current_docid << " (appearance = " << this->appearance_ << ", " <<
                        "end_flag = " << (this->end_flag ? "true" : "false") << ")");
            }
        }

        void lower_bound_merge(docid_t min_docid) {
            if(end_flag) return;
            bool got_timeout = false; //timeout.deadline();
            if(got_timeout || !merge(ranges, current_docid, min_docid, appearance_, merge_aux)) {
                next_docid = current_docid = no_docid;
                appearance_ = 0;
                end_flag = true;
            }
            if(got_timeout) {
                log_debug_hard(pi_type << ".lower_bound(" << min_docid << ")  = timeout ("
                        << this->timeout.deadline_description() << ")");
            } else {
                log_debug_mare(pi_type << ".lower_bound(" << min_docid << ") = " <<
                        this->current_docid << " (appearance = " << this->appearance_ << ", " <<
                        "end_flag = " << (this->end_flag ? "true" : "false") << ")");
            }
        }

        void lower_bound_merge_sylls(docid_t min_docid) {
            // TODO: redesign this, became too big
            
            docid_t __attribute__((unused)) _min_docid = min_docid;

            if(end_flag || ranges.empty()) return;
            if(false /*timeout.deadline()*/) {
                next_docid = current_docid = no_docid;
                appearance_ = 0;
                end_flag = true;
                log_debug_hard(pi_type << ".lower_bound(" << min_docid << ")  = timeout ("
                        << this->timeout.deadline_description() << ")");
                return;
            }

            if(corresponding_source_locator.empty() && !ranges.empty()) {
                typename source_type::const_iterator j = this->source_container().begin();
                for(typename pi_ranges::iterator i = ranges.begin(); i != ranges.end(); ++i, ++j)
                    corresponding_source_locator[i] = j;
            }

            min_docid = this->lower_bound_merge_sylls_accurate_min(min_docid);
            if(end_flag) {
                log_debug_mare(pi_type << ".lower_bound(" << min_docid << ") = " <<
                    this->current_docid << " (appearance = " << this->appearance_ << ", " <<
                    "end_flag = " << (this->end_flag ? "true" : "false") << ")");
                return;
            }

            while(true) {

                next_docid = current_docid = no_docid;
                size_t last_syll_id = 0;
                appearance_ = 0;

                auto sylls_modificator_first = [&] (const typename pi_ranges::iterator &i) -> bool {
                    current_docid = i->front();
                    typename corresponding_source_locator_t::iterator ji = corresponding_source_locator.find(i);
                    assert(ji != corresponding_source_locator.end());
                    const synonym_info &current_syninfo = *(ji->second);
                    if((pi_type == fa_main_group_iterator_e || pi_type == fa_other_group_iterator_e) &&
                            current_syninfo.begin_token > last_syll_id) return false;
                    appearance_ = (last_syll_id = current_syninfo.end_token) - current_syninfo.begin_token;
                    return true;
                };

                auto sylls_modificator_second = [&] (bool equal, const typename pi_ranges::iterator &i) -> bool {
                    const docid_t &front_docid = i->front();
                    typename corresponding_source_locator_t::iterator ji = corresponding_source_locator.find(i);
                    assert(ji != corresponding_source_locator.end());
                    const synonym_info &current_syninfo = *(ji->second);
                    if((pi_type == fa_main_group_iterator_e || pi_type == fa_other_group_iterator_e) &&
                            current_syninfo.begin_token > last_syll_id) {
                        next_docid = current_docid = no_docid;
                        last_syll_id = 0;
                        return false;
                    }
                    if(equal && current_syninfo.end_token > last_syll_id) {
                        appearance_ += current_syninfo.end_token -
                            std::max(current_syninfo.begin_token, last_syll_id);
                        last_syll_id = current_syninfo.end_token;
                    } else if(current_docid > front_docid) {
                        next_docid = current_docid;
                        current_docid = front_docid;
                        appearance_ = (last_syll_id = current_syninfo.end_token) - current_syninfo.begin_token;
                    } else if(!equal &&
                            (next_docid == no_docid || (next_docid > front_docid && current_docid < front_docid))) {
                        next_docid = front_docid;
                    }
                    return true;
                };

                auto sylls_modificator_pair = std::pair<decltype(sylls_modificator_first),
                     decltype(sylls_modificator_second)> (sylls_modificator_first, sylls_modificator_second);

                merge_custom_modificator(ranges, current_docid, min_docid, appearance_, merge_aux,
                        sylls_modificator_pair, ranges.size());

                if(next_docid == no_docid) next_docid = current_docid;

                if(current_docid == no_docid) {
                    appearance_ = 0;
                    end_flag = true;
                } else if(pi_type == fa_main_group_iterator_e || pi_type == fa_other_group_iterator_e) {
                    if(!lower_bound_merge_sylls_complete_check(last_syll_id)) {
                        min_docid = std::max(min_docid + 1, next_docid);
                        continue;
                    }
                }

                break;
            }

            log_debug_mare(pi_type << ".lower_bound(" << _min_docid << ") = " <<
                    this->current_docid << " (appearance = " << this->appearance_ << ", " <<
                    "end_flag = " << (this->end_flag ? "true" : "false") << ")");
        }

        virtual bool lower_bound_merge_sylls_complete_check(size_t) { return true; }

        virtual docid_t lower_bound_merge_sylls_accurate_min(docid_t min_docid) { return min_docid; }

        public:
        size_t appearance() const { return appearance_; }

        size_t summ_of_subappearances() const {
            size_t summ = 0;
            for(typename pi_ranges::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
                summ += i->begin().appearance();
            return summ;
        }

        virtual void predicted_increment(size_t &groups, const docid_t &current_min, docid_t &predicted) const {
            if(groups) {
                --groups;
                docid_t spinner = current_docid;
                if(spinner != no_docid && spinner <= current_min) spinner = next_docid;
                if(spinner == no_docid) return;
                if(spinner > current_min) {
                    log_debug_mare(pi_type << ".predicted_increment(" << current_min << ") = " << spinner); }
                if(predicted == no_docid || spinner < predicted) predicted = spinner;
            }
        }

        docid_t next() const { return next_docid; }

        const source_type &source_container() const { return *source; }

        virtual draft_score_t score(const docid_t &) { return draft_score_t(0); }

        virtual void fill_score_table(const docid_t &, syll_draft_score_t &) {}

        size_t scanned() const {
            size_t scnd = 0;
            for(typename pi_ranges::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
                scnd += i->begin().scanned();
            return scnd;
        }

        size_t size() const {
            if(ranges.empty()) return 0;
            size_t sz = this->ilogic_type() == intersection_type ? ((size_t) 0) - 1 : 0;
            for(typename pi_ranges::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
                if(this->ilogic_type() == merge_type)
                    sz += i->begin().size();
                else if(this->ilogic_type() == intersection_type)
                    sz = std::min(i->begin().size(), sz);
                else assert(false);
            return sz;
        }

        protected:
        draft_score_t score_synonym_info(const docid_t &docid) {
            if(current_docid != docid)
                return draft_score_t();
            if(score_cache_docid == docid)
                return score_cache_value;
            if(appearance_ != ranges.size()) {
                score_cache_docid = docid;
                score_cache_value = 0;
                return score_cache_value;
            }
            rank_t min_rank = no_rank;
            for(typename pi_ranges::iterator i = ranges.begin(); i != ranges.end(); ++i) {
                if(i->empty()) continue;
                if(min_rank == no_rank || min_rank > i->begin()->rank)
                    min_rank = i->begin()->rank;
            }
            score_cache_value = min_rank == no_rank ? 0 :
                ((draft_score_t) (source->size() * source->score * source->idf * 16))
                    * ((draft_score_t) min_rank + 1);
            if(score_cache_value > java_max_int)
                score_cache_value = java_max_int;
            score_cache_docid = docid;
            return score_cache_value;
        }

        void fill_score_table_using_synonym_infos(const docid_t &docid, syll_draft_score_t &syll_draft_score) {
            if(docid == no_docid) return;
            for(typename pi_ranges::iterator i = ranges.begin(); i != ranges.end(); ++i) {
                if(i->empty()) continue;
                draft_score_t score = i->begin().score(docid);
                if(score > draft_score_t(0))
                    for(size_t j = i->begin().source_container().begin_token;
                            j < i->begin().source_container().end_token; ++j) {
                        assert(j < syll_draft_score.size());
                        if(syll_draft_score[j] < score)
                            syll_draft_score[j] = score;
                    }
            }
        }

        virtual iterator_logic_type ilogic_type() const = 0;

        protected:
        docid_t current_docid;
        docid_t next_docid;
        bool end_flag;
        pi_ranges ranges;
        const source_type *source;
        size_t appearance_;
        docid_t score_cache_docid;
        draft_score_t score_cache_value;
        eger::timer timeout;
        merge_aux_t merge_aux;
        corresponding_source_locator_t corresponding_source_locator;
        intersection_map_t intersection_map;
        phrase_iterator_extra<pi_type, pt, d, index_c> extra;
    };

// syntactic sugar

#define define_pi_subiterator(subiter) \
    typename subiter<pt, d, index_c>::type

#define define_pi_subiterator_index_range \
    typename index_c::template range<pt, d>::value::iterator

#define define_pi_source_type(x) x

#define use_pi_make_range_m \
    protected: \
    virtual pi_range \
    source_item_to_range(index_c &ind, const typename source_type::value_type &t) { \
        return pi_range(ind, t, this->timeout); \
    } \

#define use_pi_posting_iterator_range_m \
    protected: \
    virtual pi_range \
    source_item_to_range(index_c &ind, const typename source_type::value_type &t) { \
        return ind.template posting_iterator_range<pt, d>(t.term); \
    } \

#define use_pi_lower_bound_merge_m \
    protected: \
    virtual iterator_logic_type ilogic_type() const { return base_class::merge_type; } \
    \
    public: \
    virtual void lower_bound(docid_t min_docid) { \
        return this->lower_bound_merge(min_docid); \
    } \

#define use_pi_lower_bound_merge_sylls_m \
    protected: \
    virtual iterator_logic_type ilogic_type() const { return base_class::merge_type; } \
    \
    public: \
    virtual void lower_bound(docid_t min_docid) { \
        return this->lower_bound_merge_sylls(min_docid); \
    } \

#define use_pi_lower_bound_intersection_m \
    protected: \
    virtual iterator_logic_type ilogic_type() const { return base_class::intersection_type; } \
    \
    public: \
    virtual void lower_bound(docid_t min_docid) { \
        return this->lower_bound_intersection(min_docid); \
    } \

#define use_pi_lower_bound_intersection_first_m \
    protected: \
    virtual iterator_logic_type ilogic_type() const { return base_class::intersection_type; } \
    \
    public: \
    virtual void lower_bound(docid_t min_docid) { \
        return this->lower_bound_intersection(min_docid, 1); \
    } \

#define use_pi_score_synonym_info_m \
    public: \
    virtual draft_score_t score(const docid_t &docid) { \
        return this->score_synonym_info(docid); \
    } \

#define use_pi_fill_score_table_underlaying_fill_m \
    public: \
    virtual void  \
    fill_score_table(const docid_t &docid, syll_draft_score_t &syll_draft_score) { \
        for(typename pi_ranges::iterator i = this->ranges.begin(); i != this->ranges.end(); ++i) { \
            if(i->empty()) continue; \
            i->begin().fill_score_table(docid, syll_draft_score); \
        } \
    } \

#define use_pi_fill_score_table_using_synonym_infos_m \
    public: \
    virtual void  \
    fill_score_table(const docid_t &docid, syll_draft_score_t &syll_draft_score) { \
        return this->fill_score_table_using_synonym_infos(docid, syll_draft_score); \
    } \

#define use_pi_phrase_predicted_increment_m \
    public: \
    virtual void \
    predicted_increment(size_t &groups, const docid_t &current_min, docid_t &predicted) const { \
        if(!groups) return; \
        for(typename pi_ranges::const_iterator i = this->ranges.begin(); \
                i != this->ranges.end(); ++i) \
            i->begin().predicted_increment(groups, current_min, predicted); \
    } \

#define use_pi_lower_bound_merge_sylls_accurate_min_m \
    protected: \
    virtual docid_t lower_bound_merge_sylls_accurate_min(docid_t min_docid) { \
        itim::lower_bound(this->extra.msynonym_range, min_docid); \
        if(this->extra.msynonym_range.empty()) { \
            this->next_docid = this->current_docid = no_docid; \
            this->appearance_ = 0; \
            this->end_flag = true; \
        } \
        return this->end_flag ? min_docid : this->extra.msynonym_range.front(); \
    } \

#define use_pi_lower_bound_merge_sylls_complete_check_m \
    protected: \
    virtual bool lower_bound_merge_sylls_complete_check(size_t last_syll_id) { \
        if(last_syll_id == this->source->syllable_infos.size()) { \
            this->appearance_ = this->source->syllable_infos.size(); \
            return true; \
        } \
        return false; \
    } \

#define use_pi_msynonym_extra_constructor_m \
    protected: \
    virtual void extra_constructor(index_c &ind, const source_type &source) { \
        this->extra.msynonym_range.begin() = \
            typename main_synonym_iterator<pt, d, index_c>::type(ind, source, this->timeout); \
    } \

#define use_pi_constructor_m(name) \
    public: \
    name(index_c &ind, const source_type &_source, eger::timer _timeout) : \
        phrase_iterator<name##_e, pt, d, index_c>(_source, _timeout) { \
        this->initialize(ind); \
    }

#define use_pi_empty_constructor_m(name) \
    public: \
    name() : phrase_iterator<name##_e, pt, d, index_c>() {}

#define use_pi_legacty_type_m(name) \
    public: \
    typedef name type; \
    typedef phrase_iterator<name##_e, pt, d, index_c> base_class; \
    typedef typename name::source_type source_type; \
    typedef typename name::pi_range pi_range; \
    typedef typename name::pi_ranges pi_ranges; \
    typedef typename name::sub_iterator sub_iterator; \
    typedef typename name::iterator_logic_type iterator_logic_type; \
    \
    bool operator==(const type &other) const { return base_class::equal(other); } \

#define define_phrase_iterator_header(name, subiter, src_type) \
    template<pass_type pt, depth d, class index_c> \
    struct phrase_iterator_types<name##_e, pt, d, index_c> {  \
        typedef subiter sub_iterator; \
        typedef itim::iterator_range<sub_iterator> pi_range; \
        typedef src_type source_type; \
    }; \
    \
    template<pass_type pt, depth d, class index_c> \
    class name : public phrase_iterator<name##_e, pt, d, index_c> \

#define define_phrase_iterator_extra_data_header(name) \
    template<pass_type pt, depth d, class index_c> \
    struct phrase_iterator_extra<name##_e, pt, d, index_c> \

}

#endif
