#ifndef ITIM_ALGO_H
#define ITIM_ALGO_H

#include <locale>
#include <tuple>
#include <boost/locale.hpp>
#include <srch/itim_base.h>
#include <srch/globals.h>
#include <srch/location_repos.h>
#include <srch/web_page_info.h>

namespace itim {

inline depth next_depth(depth s) {
    switch(s) {
        case first_block: return top_blocks;
        case top_blocks: return all_blocks;
        case all_blocks: break;
    }
    throw exception("Out of depths") << where(__PRETTY_FUNCTION__);
}

template<depth d> struct depth_op              { enum { next }; };
template<>        struct depth_op<first_block> { enum { next = top_blocks }; };
template<>        struct depth_op<top_blocks>  { enum { next = all_blocks }; };
template<>        struct depth_op<all_blocks>  { enum { next = all_blocks }; };
#define next_depth_tmpl(x) ((depth) depth_op<x>::next)

inline bool is_in_preferred_location(const location& pref_loc, const web_page_info& wpi) {
    if(!pref_loc || !wpi) return true;
    for(auto i = wpi->region_by_phone().begin(); i != wpi->region_by_phone().end(); ++i) {
        if(pref_loc.id == *i)
            return true;
    }
    return false;
}

template<class range_t, class value_t>
inline void lower_bound(range_t &r, const value_t &v) {
    if(!r.empty() && r.front() < v)
        r.begin().lower_bound(v);
    if(!r.empty()) assert(r.front() >= v);
}

template<class range_t, class value_t>
inline const value_t &lower_bound_return(range_t &r, const value_t &v, value_t &def_value) {
    if(!r.empty() && r.front() < v)
        lower_bound(r, v);
    return r.empty() ? def_value : r.front();
}

template<class range_container_t, class value_t, class isection_map_t>
inline bool intersection(range_container_t &rc, value_t &result,
        const value_t &initial, isection_map_t &isection_map,
        size_t rc_size = size_t(0) - 1) {

    if(rc.empty()) return false;
    if(rc_size == size_t(0) - 1) rc_size = rc.size();

    if(isection_map.empty()) {
        for(typename range_container_t::iterator i = rc.begin(); rc_size > 0; ++i, --rc_size) 
            if(i->empty()) continue;
            else isection_map.insert(typename isection_map_t::value_type(i->begin().size(), i));
        if(isection_map.empty()) return false;
    }

    value_t next_candidate = initial;
    for(typename isection_map_t::iterator i = isection_map.begin(); i != isection_map.end();) {
        lower_bound(*(i->second), next_candidate);
        if(i->second->empty()) return false;
        if(i->second->front() > next_candidate) {
            next_candidate = i->second->front();
            i = isection_map.begin();
        } else ++i;
    }

    result = next_candidate;
    return true;
}

template<class range_container_t, class value_t, class merge_aux>
inline bool merge(range_container_t &rc, value_t &result,
        const value_t &initial, size_t &appearence, merge_aux &ma,
        size_t rc_size = size_t(0) - 1) {
    if(rc.empty()) return false;
    if(rc_size == size_t(0) - 1) rc_size = rc.size();
    auto selector_first = [&] (const typename range_container_t::iterator &i) -> bool {
        result = i->front();
        appearence = 1;
        return true;
    };
    auto selector_second = [&] (bool equal, const typename range_container_t::iterator &i) -> bool {
        if(equal) ++appearence;
        else if(i->front() < result) {
            result = i->front();
            appearence = 1;
        }
        return true;
    };
    auto selectors_pair = std::pair<decltype(selector_first), decltype(selector_second)>
        (selector_first, selector_second);
    return rc_size < 9 ? 
        merge_linear(rc, result, initial, rc_size, selectors_pair) :
        merge_heap(rc, result, initial, ma, rc_size, selectors_pair);
}

template<class range_container_t, class value_t, class merge_aux, class candidate_handler_t>
inline bool merge_custom_modificator(range_container_t &rc, value_t &result,
        const value_t &initial, size_t &, merge_aux &ma,
        candidate_handler_t &candidate_handler,
        size_t rc_size = size_t(0) - 1) {
    if(rc.empty()) return false;
    if(rc_size == size_t(0) - 1) rc_size = rc.size();
    return rc_size < 18 ? 
        merge_linear(rc, result, initial, rc_size, candidate_handler) :
        merge_heap(rc, result, initial, ma, rc_size, candidate_handler);
}

template<class range_container_t, class value_t, class candidate_handler_t>
inline bool merge_linear(range_container_t &rc, value_t &result,
        value_t initial, size_t rc_size,
        candidate_handler_t &candidate_handler) {
    bool found = false;
    for(typename range_container_t::iterator i = rc.begin(); rc_size > 0; ++i, --rc_size) {
        lower_bound(*i, initial);
        if(i->empty()) continue;
        bool good = found ? candidate_handler.second(i->front() == result, i) :
            candidate_handler.first(i);
        if(!good) initial = ((value_t) i->front()) + 1;
        found = true;
    }
    return found;
}

template<class range_container_t, class value_t, class merge_aux,
    class candidate_handler_t>
inline bool merge_heap(range_container_t &rc, value_t &result,
        value_t initial, merge_aux &ma, size_t rc_size,
        candidate_handler_t &candidate_handler) {
    auto pv_to_pq = [&] () -> void {
        for(auto pi = ma.pv.begin(); pi != ma.pv.end(); ++pi) {
            lower_bound(**pi, initial);
            if((*pi)->empty()) continue;
            ma.pq.push(typename decltype(ma.pq)::value_type((*pi)->front(), (*pi)));
        }
        ma.pv.clear();
    };
    if(!ma.pv.empty()) pv_to_pq();
    if(ma.pq.empty()) {
        for(typename range_container_t::iterator i = rc.begin(); rc_size > 0; ++i, --rc_size) {
            lower_bound(*i, initial);
            if(i->empty()) continue;
            ma.pq.push(typename decltype(ma.pq)::value_type(i->front(), i));
        }
        if(ma.pq.empty()) return false;
    }
    auto descum = [&] () -> typename range_container_t::iterator {
        typename range_container_t::iterator i = ma.pq.top().second;
        ma.pq.pop();
        lower_bound(*i, initial);
        return i;
    };
    bool good = true;
    while(true) {
        if(!good) pv_to_pq();
        while(!ma.pq.empty() && ma.pq.top().first < initial) {
            typename range_container_t::iterator i = descum();
            if(!i->empty()) ma.pq.push(typename decltype(ma.pq)::value_type(i->front(), i));
        }
        if(ma.pq.empty()) return false;
        good = candidate_handler.first(ma.pq.top().second);
        if(!good) { initial = ma.pq.top().first + 1; continue; }
        ma.pv.push_back(descum());
        while(good && !ma.pq.empty() && ma.pq.top().first == result) {
            good = candidate_handler.second(true, ma.pq.top().second);
            ma.pv.push_back(descum());
        }
        if(!good) { initial = ma.pq.top().first + 1; continue; }
        if(!ma.pq.empty()) candidate_handler.second(false, ma.pq.top().second); // for next_docid
        break;
    }
    return true;
}

template<class string_t>
string_t uppercase(const string_t &s) {
    return boost::locale::to_upper(s, globals::get_locale());
}

template<class string_t>
string_t lowercase(const string_t &s) {
    return boost::locale::to_lower(s, globals::get_locale());
}

template<class string_t>
bool icompare(const string_t &s, const string_t &d) {
    using namespace std;
    using namespace boost::locale;
    return !use_facet<collator<char> >(globals::get_locale()).compare(collator_base::secondary, s, d);
}

template<class string_t>
bool icompare_less(const string_t &s, const string_t &d) {
    using namespace std;
    using namespace boost::locale;
    return use_facet<collator<char> >(globals::get_locale()).compare(collator_base::secondary, s, d) == -1;
}

template<class string_t>
bool icompare_greater(const string_t &s, const string_t &d) {
    using namespace std;
    using namespace boost::locale;
    return use_facet<collator<char> >(globals::get_locale()).compare(collator_base::secondary, s, d) == 1;
}



}

#endif
