#ifndef ITIM_TYPES_TO_STREAM_H
#define ITIM_TYPES_TO_STREAM_H

#include <ostream>
#include <srch/itim_types.h>

namespace itim {

    inline std::ostream &operator<<(std::ostream &s, docid_t d) {
        s << '{';
        if(d.bucketid == no_bucketid) s << "no_bucketid";
        else s << d.bucketid;
        s << ',';
        if(d.local_docid == no_local_docid) s << "no_local_docid";
        else s << d.local_docid;
        s << '}';
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, depth d) {
        switch(d) {
            case first_block: s << "first_block"; break;
            case top_blocks: s << "top_blocks"; break;
            case all_blocks: s << "all_blocks"; break;
        }
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, pass_type pt) {
        switch(pt) {
            case draft: s << "draft"; break;
            case full: s << "full"; break;
        }
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, appearance app) {
        switch(app) {
            case usual_appearance: s << "usual_appearance"; break;
            case full_appearance: s << "full_appearance"; break;
        }
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, block b) {
        switch(b) {
            case b1k: s << "b1k"; break;
            case b100k: s << "b100k"; break;
            case btail: s << "btail"; break;
        }
        return s;
    }

    inline std::ostream &operator<<(std::ostream &s, query_phrase phrase) {
        s << "qphrase = ";
        for (auto i = phrase.begin(); i != phrase.end(); ++i) {
            s << "{ ";
            for (auto ii = i->begin(); ii != i->end(); ++ii) {
                s << "terms = ";
                for (auto iii = ii->begin(); iii != ii->end(); ++iii) {
                    s << iii->term;
                    if ( iii + 1 != ii->end() ) {
                        s << ", ";
                    } else {
                        s << "; ";
                    }
                }
                s << "basic_idf = " << ii->basic_idf << "; ";
                s << "idf = " << ii->idf << "; ";
                s << "score = " << ii->score << "; ";
            }
            s << " }" << std:: endl;
        }
        return s;
    }

}

#endif
