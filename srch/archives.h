#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <algorithm>
#include <stdint.h>
#include <srch/itim_base.h>
#include <srch/itim_iterator_range.h>

namespace itim {

template<class base_rw_iterator, class block_type>
class bit_byte_read {
    public:
    bit_byte_read() :
        cache(0),
        range(NULL),
        bit_offset(sizeof(block_type) * 8)
        {} // for empty codecs

    bit_byte_read(itim::iterator_range<base_rw_iterator> &_range,
            size_t _bit_offset = 0) :
        range(&_range)
    {
        initialize_mask();
        align_by_block(_bit_offset);
    }

    bit_byte_read(itim::iterator_range<base_rw_iterator> &_range,
            const bit_byte_read<base_rw_iterator, block_type> &other) :
        cache(other.cache),
        range(&_range),
        bit_offset(other.bit_offset)
    { memcpy(bit_byte_mask, other.bit_byte_mask, sizeof(bit_byte_mask)); }

    private:
    void initialize_mask() {
        for(block_type i = 1, o = sizeof(block_type) * 8; i; i <<= 1)
            bit_byte_mask[--o] = i;
    }

    public:
    void init(itim::iterator_range<base_rw_iterator> &_range, size_t _bit_offset = 0) {
        range = &_range;
        initialize_mask();
        align_by_block(_bit_offset);
    }

    size_t reading_offset_bits() const {
        return (range->begin().byte_offset() * 8) -
            (__builtin_expect(bit_offset == sizeof(block_type) * 8, 0) ? 0 : (sizeof(block_type) * 8 - bit_offset));
    }

    void new_position(base_rw_iterator &base, size_t _bit_offset) {
        range->begin() = base;
        range->begin() += _bit_offset / 8;
        align_by_block(_bit_offset % 8);
    }

    inline bool next_bit() {
        if(__builtin_expect(bit_offset == sizeof(block_type) * 8, 0)) fill_cache();
        return cache & bit_byte_mask[bit_offset++];
    }


    inline uint64_t next_nbit(uint64_t n) {
        uint64_t r = 0;

        if(__builtin_expect(n == 8 && bit_offset % 8 == 0, 0))
            return next_byte();

        while(__builtin_expect(n > 0, 1)) {
            if(__builtin_expect(bit_offset == sizeof(block_type) * 8, 0)) fill_cache();
            size_t bits_in_current_block = sizeof(block_type) * 8 - bit_offset;
            size_t bits_to_read = std::min((size_t) n, bits_in_current_block);
            r = (r << bits_to_read) |
                (((cache << bit_offset) & (block_type) (block_type(0) - 1)) >> (sizeof(block_type) * 8 - bits_to_read));
            bit_offset += bits_to_read;
            n -= bits_to_read;
        }

        return r;
    }

    inline uint8_t next_byte() {
        bit_offset &= 0xf8;
        if(__builtin_expect(bit_offset == sizeof(block_type) * 8, 0)) fill_cache();
        uint8_t r = (cache >> (sizeof(block_type) * 8 - 8 - bit_offset));
        bit_offset += 8;
        return r;
    }

    inline size_t skip_unary() {
        if(__builtin_expect(bit_offset == sizeof(block_type) * 8, 0)) fill_cache();
        if(__builtin_expect(bit_offset, 1))
            cache &= (((block_type) 1 << (sizeof(block_type) * 8 - bit_offset)) - 1);
        size_t count = 0 - bit_offset;
        while(__builtin_expect(!cache, 0)) {
            count += sizeof(block_type) * 8;
            fill_cache();
        }
        size_t bits_to_skip = 0;
        switch(sizeof(block_type)) {
            case 1 : bits_to_skip = __builtin_clz(cache) - 3*8; break;
            case 4 : bits_to_skip = __builtin_clz(cache); break;
            case 8 : bits_to_skip = __builtin_clzl(cache); break;
        }
        bit_offset = bits_to_skip + 1;
        return count + bits_to_skip;
    }

    inline size_t count_null_bits() {
        return skip_unary();

        /*
        size_t q = 0;
        while(!next_bit()) ++q;
        return q;
        */

        /* another (slower) realization
        size_t in = ((size_t) &(range->front())) * 8 + bit_offset;
        skip_unary();
        size_t diff = ((size_t) &(range->front())) * 8 + bit_offset - in;
        return diff - 1;
        */
    }

    inline void align_by_byte() {
        if( bit_offset % 8 ) (bit_offset &= (~ 7)) += 8;
    }

    inline void align_by_long() {
        if( bit_offset % 64 ) (bit_offset &= (~ 63)) += 64;
    }

    template <class input_type, class return_type>
    inline return_type java_convert(input_type in) {
        union convert_bits {
            input_type in_data;
            return_type out_data;
        } bits;

        bits.in_data = in;
        return bits.out_data;
    }

    private:
    inline void fill_cache() {
        cache = *((block_type*) &(range->front()));
        range->advance_begin(sizeof(block_type));
        // switch(sizeof(block_type)) {
        //     case 1 : break;
        //     case 4 : cache = __builtin_bswap32(cache); break;
        //     case 8 : cache = __builtin_bswap64(cache); break;
        // }
        bit_offset = 0;
    }

    inline void align_by_block(size_t bo) {
       size_t ptr = (size_t) range->begin().pointer();
       ptr <<= 3;
       ptr += bo;
       size_t block_bit_offset = ptr % (sizeof(block_type) * 8);
       if(__builtin_expect(block_bit_offset == 0, 0)) {
           ptr >>= 3;
           ptr -= (size_t) range->begin().pointer();
           if(ptr) range->advance_begin((std::streamoff) ptr);
           bit_offset = sizeof(block_type) * 8;
       } else {
            ptr -= block_bit_offset;
            ptr >>= 3;
            std::streamoff move = (std::streamoff) ptr - (size_t) range->begin().pointer();
            if(move) range->advance_begin(move);
            fill_cache();
            bit_offset = block_bit_offset;
       }
    }

    private:
    block_type cache;
    itim::iterator_range<base_rw_iterator> *range;
    size_t bit_offset;
    block_type bit_byte_mask[sizeof(block_type) * 8];
};

template<class base_rw_iterator, class block_type>
class elias  {
    public:
    elias(bit_byte_read<base_rw_iterator, block_type> &_bbr) : bbr(&_bbr) {}

    template<class return_type>
    return_type gamma() {
        size_t m = bbr->count_null_bits();

        return_type v = __builtin_expect(m, 1) ? (((return_type) 1) << m) : 1;
        if(__builtin_expect(m == 1, 0)) v |= (return_type) bbr->next_bit();
        else if(__builtin_expect(m, 1)) v |= (return_type) bbr->next_nbit(m);

        return v;
    }

    template<class return_type>
    return_type delta() {
        size_t m = gamma<size_t>();
        return_type v = __builtin_expect(m, 1) ? (((return_type) 1) << m) : 1;

        if(__builtin_expect(m == 1, 0)) v |= (return_type) bbr->next_bit();
        else if(__builtin_expect(m, 1)) v |= (return_type) bbr->next_nbit(m);

        return v;
    }

    private:
    bit_byte_read<base_rw_iterator, block_type> *bbr;
};

constexpr size_t compile_log2(size_t k, size_t p = 0) {
    return k/2 ? compile_log2(k/2, p + 1) : p;
}

template<class base_rw_iterator, class block_type>
class golomb  {
    public:
    golomb(bit_byte_read<base_rw_iterator, block_type> &_bbr) :
        bbr(&_bbr)
    {}

    template<class return_type, size_t k>
    return_type read() {
        return_type r = 0, d;
        size_t log2k = compile_log2(k);
        size_t j = log2k;

        d = ((return_type) 1 << (log2k + 1)) - k;
        return_type q = bbr->count_null_bits();
        if(__builtin_expect(j == 1, 0)) r = bbr->next_bit();
        else if(__builtin_expect(j, 1)) r = bbr->next_nbit(j);
        if(r >= d) r = ((r << 1) | (return_type) bbr->next_bit()) - d;

        return q * k + r;
    }

    private:
    bit_byte_read<base_rw_iterator, block_type> *bbr;
};

template <class base_rw_iterator, class block_type>
class variable_byte  {
    public:
    variable_byte(bit_byte_read<base_rw_iterator, block_type> &_bbr) :
        bbr(&_bbr)
    {}

    template<class return_type>
    return_type read_vle() {
        return_type v = 0;
        uint8_t i = 0x80;
        uint8_t shift = 0;

        while(__builtin_expect(i & 0x80, 1))
            v |= (((return_type) (i = bbr->next_byte())) & 0x7f) << (shift++ * 7);

        return v;
    }

    template<class return_type>
    return_type read_7bit() {
        return_type v = 0;
        uint8_t i = 0x1;

        while(__builtin_expect(i & 0x1, 1))
            v = (v << 7) | ((i = bbr->next_byte()) >> 1);

        return v;
    }

    template<class return_type>
    return_type read_nbits(size_t nbits) {
        if(nbits == 7) return read_7bit<return_type>();
        assert(nbits < sizeof(size_t)*8);

        return_type v = 0;
        size_t i = 0x1;

        while(__builtin_expect(i & 0x1, 1))
            v = (v << nbits) | ((i = bbr->next_nbit(nbits + 1)) >> 1);

        return v;
    }

    private:
    bit_byte_read<base_rw_iterator, block_type> *bbr;
};

template <class base_rw_iterator, class block_type>
class truncated_binary  {
    public:
    truncated_binary(bit_byte_read<base_rw_iterator, block_type> &_bbr) :
        bbr(&_bbr)
    {}

    template<class return_type, size_t n>
    return_type read() {
        return_type v = 0;
        size_t k = compile_log2(n);
        if(!k) return v;

        return_type u = (((return_type) 1) << (k + 1)) - n;
        if(__builtin_expect(k == 1, 0)) v = bbr->next_bit();
        else if(__builtin_expect(k, 1)) v = bbr->next_nbit(k);
        if(v >= u) v = ((v << 1) | bbr->next_bit()) - u;

        return v;
    }

    private:
    bit_byte_read<base_rw_iterator, block_type> *bbr;
};

template <class base_rw_iterator, class block_type>
class utf8  {
    public:
    utf8(bit_byte_read<base_rw_iterator, block_type> &_bbr) :
        bbr(&_bbr)
    {}

    template<class container>
    void read(container &s, size_t n) {
        while(__builtin_expect(n--, 1)) {
            uint8_t c = bbr->next_byte();
            s.push_back(c);
            switch(c) {
                case 0x80 ... 0xdf: s.push_back(bbr->next_byte()); break;
                case 0xe0 ... 0xef: s.push_back(bbr->next_byte());
                                    s.push_back(bbr->next_byte()); break;
                case 0xf0 ... 0xff: throw exception("UTF-8 unsupported symbol") <<
                                    where(__PRETTY_FUNCTION__);
            }
        }
    }

    template<class container>
    size_t length_in_bytes(const container &s, size_t symbols) {
        size_t r = 0;
        typename container::const_iterator i = s.begin();
        while(__builtin_expect(i != s.end(), 1)) {
            if(! ((((uint8_t) (*i)) & 0xc0) == 0x80))
                if(symbols-- == 0) break;
            ++r;
            ++i;
        }
        return r;
    }

    private:
    bit_byte_read<base_rw_iterator, block_type> *bbr;
};

}

#endif
