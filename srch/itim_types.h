#ifndef ITIM_TYPES_H
#define ITIM_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <vector>
#include <set>
#include <assert.h>
#include <srch/itim_string.h>
#include <srch/itim_constants.h>

namespace itim {

    enum depth { first_block, top_blocks, all_blocks };
    enum block { b1k, b100k, btail };
    enum pass_type { draft, full };
    enum appearance { usual_appearance, full_appearance };

    // typedef uint64_t docid_t;
    typedef uint64_t siteid_t;
    typedef uint32_t bucketid_t;
    typedef uint32_t local_docid_t;
    typedef uint32_t rank_t;
    typedef uint32_t zoneid_t;
    typedef uint32_t position_t;
    typedef uint32_t alienid_t;
    typedef int64_t offset_t;
    typedef uint64_t draft_score_t;

    typedef std::vector<size_t> syll_draft_score_t;

    typedef string term_type;

    const rank_t no_rank = rank_t(0) - 1;
    const zoneid_t no_zoneid = zoneid_t(0) - 1;
    const siteid_t no_siteid = siteid_t(0) - 1;
    const local_docid_t no_local_docid = local_docid_t(0) - 1;
    const bucketid_t no_bucketid = bucketid_t(0) - 1;

    typedef size_t aindex_t;
    const aindex_t not_exists = aindex_t(0) - 1;

    struct docid_t;
    docid_t docid_summ_union(docid_t, size_t);

#pragma pack(push, 1)
    struct docid_t {
        local_docid_t local_docid; // to optimize by speed we are addressing to docid as to (size_t)
        bucketid_t bucketid;

        docid_t(): local_docid(no_local_docid), bucketid(no_bucketid) {}
        docid_t(const local_docid_t &ld, const bucketid_t &bid) : local_docid(ld), bucketid(bid)  {}
        docid_t(const local_docid_t &ld) : local_docid(ld), bucketid(no_bucketid) {}

        docid_t operator+(size_t d) const {
            return docid_summ_union(*this, d);
            /*docid_t n = *this; fair_add(n, d); return n;*/
        }
        docid_t &operator+=(size_t d) {
            *((uint64_t*) this) += d;
            /*fair_add(*this, d);*/
            return *this;
        }
        docid_t &operator++() {
            ++ *((uint64_t*) this);
            /*fair_add(*this, 1);*/
            return *this;
        }

        inline bool operator==(const docid_t& other) const {
            return *((uint64_t*) this) == *((uint64_t*) &other);
            /*return local_docid == other.local_docid && bucketid == other.bucketid*/;
        }

        inline bool operator!=(const docid_t& other) const  {
            return *((uint64_t*) this) != *((uint64_t*) &other);
            /*return local_docid != other.local_docid || bucketid != other.bucketid;*/
        }

        inline bool operator>(const docid_t& other)  const {
            return *((uint64_t*) this) > *((uint64_t*) &other);
            /*return bucketid > other.bucketid ||
                (bucketid == other.bucketid && local_docid > other.local_docid);*/
        }

        inline bool operator>=(const docid_t& other)  const {
            return *((uint64_t*) this) >= *((uint64_t*) &other);
            /*return bucketid > other.bucketid ||
                (bucketid == other.bucketid && local_docid >= other.local_docid)*/;
        }

        inline bool operator<(const docid_t& other)  const {
            return *((uint64_t*) this) < *((uint64_t*) &other);
            /*return bucketid < other.bucketid ||
                (bucketid == other.bucketid && local_docid < other.local_docid);*/
        }

        inline bool operator<=(const docid_t& other)  const {
            return *((uint64_t*) this) <= *((uint64_t*) &other);
            /*return bucketid < other.bucketid ||
                (bucketid == other.bucketid && local_docid <= other.local_docid);*/
        }

        private:
        void fair_add(docid_t &di, size_t d) {
            assert( di.local_docid != no_local_docid );
            bool prev_no_bucketid = di.bucketid == no_bucketid;
            if (di.local_docid == no_local_docid ) { di.local_docid = 0; }
            uint64_t t(di.bucketid);
            assert(sizeof(t) >= sizeof(local_docid_t) + sizeof(bucketid_t));
            t <<= (sizeof(local_docid_t) * 8);
            t += di.local_docid;
            t += d;
            di.local_docid = (local_docid_t) t;
            t >>= (sizeof(local_docid_t) * 8);
            di.bucketid = (bucketid_t) t;
            if(prev_no_bucketid) di.bucketid = no_bucketid;
        }
        // try to not use it
        docid_t operator++(int) { docid_t last_value = *this; ++local_docid; return last_value; }
    };
#pragma pack(pop)
    const docid_t no_docid = docid_t(no_local_docid, no_bucketid);

    union docid_speed {
        docid_speed(docid_t d) : docid(d) {}
        docid_t docid;
        uint64_t uint;
    };

    inline docid_t docid_summ_union(const docid_t did, size_t d) {
        docid_speed ds(did);
        ds.uint += d;
        return ds.docid;
    }

    //typedef std::vector<region> regions;

    struct token { // one word
        token() : name(false) {}
        token(const term_type& term_) : term(term_), name(false) {}
        term_type term;
        // QUESTION: aren't there other places for the following info?
        size_t syl_begin;
        size_t syl_end;
        double idf;
        bool name;
    };

    struct synonym_info: public std::vector<token> {
        // data
        std::vector<token> non_tone_tokens;

        // params
        bool vnese;
        double idf;
        double basic_idf;
        size_t begin_token;
        size_t end_token;
        double score;
        enum synonym_info_type {
            ORIGINAL_TYPE = 0,
            ADDTONE_TYPE = 1,
            BRAND_TYPE = 2,
            NUMBER_TYPE = 3,
            STICKY_TYPE = 4,
            MERGING_TYPE = 5,
            SHORTFORM_TYPE = 6,
            LONGFORM_TYPE = 7,
            STRONG_TYPE = 8,
            DOMAIN1_TYPE = 9,
            DOMAINPHRASE_TYPE = 10,
            NAME_TYPE = 11,
            DICTIONARY_TYPE = 12,
            ADDRESS_TYPE = 13,
            CITY_TYPE = 14,
            PHONE_TYPE = 15,
            SPELLCHECKER_TYPE = 16,
            BAD_ORIGINAL_TYPE = 17,
            PLUS_TYPE = 18,
            TRANSFORM_TYPE = 19,
            ENGLISH_TYPE = 20
        } type;

        // flags
        bool base;
        bool name;
        bool spec;
        bool conj;
        bool first_domain_level;
        bool urlsyn;
        bool city;

        synonym_info():
            base(false), name(false), spec(false), conj(false), first_domain_level(false), urlsyn(false), city(false) {}
    };

    struct syllable_info {
        string syll;
        double idf;
        size_t pos;

        syllable_info(const string& syll_, const double& idf_): syll(syll_), idf(idf_) {}
    };


    struct query_group: public std::vector<synonym_info>  {
        typedef std::vector<size_t> main_synonym_offs_t;
        main_synonym_offs_t main_synonym_offs;
        std::vector<syllable_info> syllable_infos;
        std::set<uint32_t> host_ranges;
        bool is_main;
        size_t topic_synonym_off;

        query_group(): is_main(false) {}

        bool is_weak_main_synonym() {
            for (auto i = main_synonym_offs.begin(); i != main_synonym_offs.end(); ++i) {
                if ( operator[](*i).idf < itim::strong_idf_limit )
                    return true;
            }
            return false;
        }
    };

    // search phrase is collection of syllables
    typedef std::vector<query_group> query_phrase; // set of query_groups

    typedef std::vector<term_type> minus_item;
    typedef std::vector<minus_item> minus_phrase;

    typedef std::vector<size_t> hit_list;

    // REMOVE? there's syllable_info class in query_group.h
    struct syll_info {
        syll_info(const string &_syll, size_t _pos, bool _unchange = false) :
            syll(_syll), pos(_pos), unchange(_unchange), special_token(false),
            first_domain_level(false), strong(false), weak(true) {}
        string syll;
        size_t pos;
        double idf;
        bool unchange;
        bool special_token;
        bool first_domain_level;
        bool strong;
        bool weak;
    };

    typedef std::vector<syll_info> syll_infos;
}

namespace std {
    template<>
    struct hash< itim::docid_t> {
        typedef itim::docid_t argument_type;
        typedef size_t result_type;

        result_type operator()(const argument_type& id) const {
            return id.local_docid;
        }
    };
}


#endif
