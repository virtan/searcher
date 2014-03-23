#ifndef QATAR_CODECS_H
#define QATAR_CODECS_H

#include <boost/assign/list_of.hpp>
#include <boost/array.hpp>
#include <vector>
#include <algorithm>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>
#include <srch/archives.h>
#include <srch/lexicon_item.h>
#include <srch/index_data_iterator.h>
#include <srch/posting.h>

namespace itim {

template <class index_data_accessor, class posting_type>
class qatar_index_base_codec {
    public:
    typedef index_data_iterator<index_data_accessor> ind_iter;

    public:
    qatar_index_base_codec() :
        ndoc(0), rdoc(1),
        elias_a(bit_byte_a),
        golomb_a(bit_byte_a),
        variable_byte_a(bit_byte_a),
        truncated_binary_a(bit_byte_a),
        max_docid_plus_1(0)
    {}

    qatar_index_base_codec(const lexicon_item &_lexitem, size_t _max_docid_plus_1) :
        lexitem(_lexitem),
        ndoc(0),
        rdoc(1),
        last_used_skip(0),
        bit_byte_a(),
        elias_a(bit_byte_a),
        golomb_a(bit_byte_a),
        variable_byte_a(bit_byte_a),
        truncated_binary_a(bit_byte_a),
        max_docid_plus_1(_max_docid_plus_1),
        docid_already_absolute(false)
    {}

    qatar_index_base_codec(const qatar_index_base_codec &other) :
        posting(other.posting),
        lexitem(other.lexitem),
        irange(other.irange),
        after_skiplist(other.after_skiplist),
        ndoc(other.ndoc),
        rdoc(other.rdoc),
        skip_docids(other.skip_docids),
        skip_offsets(other.skip_offsets),
        last_used_skip(other.last_used_skip),
        bit_byte_a(irange, other.bit_byte_a),
        elias_a(bit_byte_a),
        golomb_a(bit_byte_a),
        variable_byte_a(bit_byte_a),
        truncated_binary_a(bit_byte_a),
        max_docid_plus_1(other.max_docid_plus_1),
        bl(other.bl),
        docid_already_absolute(other.docid_already_absolute)
    {}

    void init(const block &_bl, index_data_accessor &ida) {
        assert(ndoc == 0 && rdoc == 1);
        bl = _bl;
        aindex_t offset = calculate_offset(lexitem, bl);
        if(offset != not_exists) {
            irange = itim::make_iterator_range(
                        ind_iter(ida, offset),
                        ind_iter(ida, ind_iter::end)
                     );
            bit_byte_a.init(irange);

            ndoc = elias_a.template gamma<size_t>() - 1;
            if(ndoc > 0) rdoc = 0;
            else rdoc = 1;
            if(ndoc > default_skip_len) {
                bit_byte_a.skip_unary();
                truncated_binary_a.template read<uint32_t, golomb_factor_skip_size>();
                bit_byte_a.align_by_long();
                // bit_byte_a.align_by_byte();
                size_t nskips = (ndoc - 1) / default_skip_len;
                skip_docids.reserve(nskips);
                skip_offsets.reserve(nskips);
                docid_t last_docid(0);
                size_t last_offset(0);
                while(nskips--) {
                    last_docid +=
                        variable_byte_a.template read_nbits<offset_t>(variable_nbits_factor_docid_delta);
                    last_offset +=
                        golomb_a.template read<offset_t, golomb_factor_skip_offset_delta>();
                    skip_docids.emplace_back(last_docid);
                    skip_offsets.emplace_back(last_offset);
                }
            }
            last_used_skip = 0;
            // bit_byte_a.align_by_byte();
            bit_byte_a.align_by_long();
            after_skiplist = ind_iter(irange.begin().data_accessor(), bit_byte_a.reading_offset_bits() / 8);
        }
        if (this->irange.begin().id() != not_exists) {
            log_debug_mare("qatar_index_codec[" << this->irange.begin().id() << "," << this->bl << "].init().ndoc = " << this->ndoc);
        } else { log_debug_mare("qatar_index_codec[" << "not_exists" << "]"); }
    }

    void increment() {
        if(rdoc > ndoc) return;
        read_block_update_skip();
    }

    void lower_bound(docid_t min_docid) {
        if(rdoc > ndoc) return;
        bool need_initial_read = false;
        if(last_used_skip != skip_docids.size()) {
            size_t new_skip;
            for(new_skip = last_used_skip; new_skip < skip_docids.size() &&
                    skip_docids[new_skip] <= min_docid; ++new_skip);
            if(new_skip != last_used_skip) {
                bit_byte_a.new_position(after_skiplist, skip_offsets[new_skip - 1]);
                posting.docid = skip_docids[new_skip - 1];
                docid_already_absolute = true;
                rdoc = new_skip * default_skip_len;
                last_used_skip = new_skip;
                need_initial_read = true;
            }
        }
        if(need_initial_read) read_block_update_skip();
        while(posting.docid < min_docid) read_block_update_skip();
    }

    bool equal(qatar_index_base_codec const &other) const {
        if(other.rdoc > other.ndoc) return rdoc > ndoc;
        if(rdoc > ndoc) return other.rdoc > other.ndoc;
        return other.irange == irange && other.lexitem == lexitem;
    }

    size_t scanned() const {
        log_debug_mare("qatar_index_codec[" << this->irange.begin().id() <<
                "," << this->bl << "].scanned().{rdoc,ndoc} = {"
                << this->rdoc << "," << this->ndoc << "}");
        return std::min(rdoc, ndoc);
    }

    size_t size() const { return ndoc; }

    protected:
    void read_block_base() {
        if(rdoc >= ndoc) posting.docid = no_docid;
        else {
            //std::cout << '(' << this->irange.begin().byte_offset() << ',' << this->bit_byte_a.bit_offset << ")\n";
            offset_t delta = variable_byte_a.template read_nbits<offset_t>(variable_nbits_factor_docid_delta);
            docid_t __attribute__((unused)) old_value = posting.docid;
            if(!docid_already_absolute) posting.docid = (posting.docid == no_docid ? docid_t(0) : posting.docid) + delta;
            else docid_already_absolute = false;
            log_debug_mare("qatar_index_codec[" << this->irange.begin().id() << "," <<
                    this->bl<< "].next_docid " <<
                    this->posting.docid << " = " << old_value << " + " << delta
                    << (this->docid_already_absolute ? " (already absolute value)" : ""));
                    // << " (" << this->lexitem.offsets[this->block_to_index(this->bl)] << ")");
            assert(posting.docid < max_docid_plus_1);
        }
    }

    void read_block_update_skip() {
        if((rdoc + 1) % default_skip_len == 0) ++last_used_skip;
        read_block();
    }

    virtual void read_block() = 0;


    private:
    size_t calculate_offset(const lexicon_item &l, const block &b) {
        return l.offsets[block_to_index(b)];
    }

    virtual size_t block_to_index(const block &b) = 0;

    protected:
    posting_type posting;

    private:
    lexicon_item lexitem;
    itim::iterator_range<ind_iter> irange;
    ind_iter after_skiplist;

    protected:
    size_t ndoc;
    size_t rdoc;

    private:
    std::vector<docid_t> skip_docids;
    std::vector<size_t> skip_offsets;
    size_t last_used_skip;

    protected:
    bit_byte_read<ind_iter, uint64_t> bit_byte_a;
    elias<ind_iter, uint64_t> elias_a;
    golomb<ind_iter, uint64_t> golomb_a;
    variable_byte<ind_iter, uint64_t> variable_byte_a;
    truncated_binary<ind_iter, uint64_t> truncated_binary_a;

    private:
    size_t max_docid_plus_1;
    block bl;
    bool docid_already_absolute;
};

template<class index_data_accessor>
class qatar_draft_codec : public qatar_index_base_codec<index_data_accessor, draft_posting> {
    public:
    typedef qatar_index_base_codec<index_data_accessor, draft_posting> base_codec;
    typedef const draft_posting& posting_type;
    typedef boost::tuple<const lexicon_item&, const block&, index_data_accessor&, size_t> initializers;

    public:
    qatar_draft_codec() : base_codec() {}

    qatar_draft_codec(const qatar_draft_codec &other) :
        base_codec(other)
    {}

    qatar_draft_codec(const initializers &in) :
        base_codec(in.template get<0>(), in.template get<3>()) {
            this->init(in.template get<1>(), in.template get<2>());
        if(this->rdoc < this->ndoc) { this->read_block_update_skip(); }
    }

    const draft_posting &dereference() const {
        return this->posting;
    }

    private:
    virtual void read_block() {
        this->read_block_base();
        if(this->rdoc >= this->ndoc) this->posting.rank = no_rank;
        else this->posting.rank = this->golomb_a.template read<rank_t, golomb_factor_draft_rank>();
        ++this->rdoc;
    }

    virtual size_t block_to_index(const block &b) {
        switch(b) {
            case b1k: return 0;
            case b100k: return 2;
            case btail: return 4;
        };
        // not reached
        return 0;
    }
};

template<class index_data_accessor>
class qatar_full_codec : virtual public qatar_index_base_codec<index_data_accessor, full_posting> {
    public:
    typedef qatar_index_base_codec<index_data_accessor, full_posting> base_codec;
    typedef const full_posting& posting_type;
    typedef boost::tuple<const lexicon_item&, const block&, index_data_accessor&, size_t> initializers;

    public:
    qatar_full_codec() :
        base_codec() {}

    qatar_full_codec(const qatar_full_codec &other) :
        base_codec(other)
    {}

    qatar_full_codec(const initializers &in) :
        base_codec(in.template get<0>(), in.template get<3>()) {
            this->init(in.template get<1>(), in.template get<2>());
        if(this->rdoc < this->ndoc) { this->read_block_update_skip(); }
    }

    const full_posting &dereference() const {
        return this->posting;
    }

    private:
    virtual void read_block() {
        this->read_block_base();
        if(this->rdoc >= this->ndoc) {
            this->posting.zones.clear();
            ++this->rdoc;
            return;
        }
        size_t nzones = this->elias_a.template gamma<size_t>();
        this->posting.zones.clear();
        this->posting.zones.reserve(nzones);
        while(nzones--) {
            this->posting.zones.emplace_back(zone());
            this->posting.zones.back().zoneid = this->elias_a.template gamma<zoneid_t>() - 1;
            bool normal_position = this->bit_byte_a.next_bit() == false;
            if(normal_position) decode_positions(this->posting.zones.back().positions());
            else decode_alien_positions(this->posting.zones.back().aliens());
        }
        ++this->rdoc;
    }

    void decode_positions(positions_t &positions) {
        size_t npos = this->elias_a.template gamma<size_t>() - 1;
        positions.clear();
        positions.reserve(npos);
        position_t position = 0;
        while(npos--)
            positions.emplace_back(position += this->golomb_a.template read<position_t, golomb_factor_position_delta>());
    }

    void decode_alien_positions(aliens_t &aliens) {
        size_t naliens = this->elias_a.template gamma<size_t>() - 1;
        aliens.clear();
#ifdef unordered_map_reserve
        aliens.reserve(naliens);
#endif
        alienid_t alienid = 0;
        while(naliens--)
            decode_positions(aliens[
                alienid += this->golomb_a.template read<alienid_t, golomb_factor_alien_position_delta>()
            ]);
    }

    virtual size_t block_to_index(const block &b) {
        switch(b) {
            case b1k: return 1;
            case b100k: return 3;
            case btail: return 5;
        };
        // not reached
        return 0;
    }
};


template <class index_data_accessor>
class qatar_lexicon_codec {
    public:
    typedef index_data_iterator<index_data_accessor> lex_iter;

    private:
    struct qatar_lexicon_codec_state {
        qatar_lexicon_codec_state() : offsets(boost::assign::list_of(0)(0)(0)(0)(0)(0)) {}
        term_type term;
        boost::array<aindex_t, 6> offsets;
    } state;

    public:
    qatar_lexicon_codec(itim::iterator_range<lex_iter> &_irange) :
        irange(_irange),
        bit_byte_a(irange),
        variable_byte_a(bit_byte_a),
        utf8_a(bit_byte_a) {}

    lexicon_item_and_term next() {
        if(irange.empty())
            throw out_of_range_exception("Out of iterator range") <<
                where(__PRETTY_FUNCTION__);
        uint32_t common_len = variable_byte_a.template read_vle<uint32_t>();
        uint32_t suffix_len = variable_byte_a.template read_vle<uint32_t>();
        size_t bytes_to_copy = utf8_a.length_in_bytes(state.term, common_len);
        term_type term(state.term, 0, bytes_to_copy);
        term.reserve(bytes_to_copy + suffix_len + /* empiric */ 2);
        utf8_a.template read(term, suffix_len);
        for(size_t i = 0; i < term.size(); ++i)
            if(term[i] == '_') term[i] = ' ';
        if(term.size() >= 3 && term.substr(term.size() - 3) == "#-1")
            term.resize(term.size() - 3);
        uint32_t df = variable_byte_a.template read_vle<uint32_t>();
        float idf = bit_byte_a.template java_convert<uint32_t, float>(
                bit_byte_a.template next_nbit(sizeof(uint32_t) * 8));
        boost::array<aindex_t, 6> offsets = { { 0, 0, 0, 0, 0, 0 } };
        for(size_t i = 0; i < 6; ++i) {
            offset_t delta_offset = variable_byte_a.template read_vle<offset_t>();
            if(delta_offset == -1) offsets[i] = not_exists;
            else state.offsets[i] = offsets[i] =
                state.offsets[i] + (delta_offset >> 3);
        }
        state.term = term;
        return lexicon_item_and_term(term, lexicon_item(df, idf, offsets));
    }

    private:
    itim::iterator_range<lex_iter> irange;
    bit_byte_read<lex_iter, uint8_t> bit_byte_a;
    variable_byte<lex_iter, uint8_t> variable_byte_a;
    utf8<lex_iter, uint8_t> utf8_a;
};

}

#endif
