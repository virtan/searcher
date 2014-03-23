#ifndef INDEX_DATA_ITERATOR_H
#define INDEX_DATA_ITERATOR_H

#include <boost/iterator/iterator_facade.hpp>
#include <stdint.h>
#include <stddef.h>
#include <srch/mapped_region.h>

namespace itim {

template<class index_data_accessor = mapped_region>
class index_data_iterator :
    public boost::iterator_facade<
        index_data_iterator<index_data_accessor>,
        uint8_t,
        boost::random_access_traversal_tag>
{
    public:
    enum end_type { end };

    public:
    index_data_iterator() :
        ida(0), offset(0) {}
    index_data_iterator(const index_data_accessor &_ida) :
        ida(&_ida), offset(0) {}
    index_data_iterator(const index_data_accessor &_ida, size_t _offset) :
        ida(&_ida), offset(_offset) {}
    index_data_iterator(const index_data_accessor &_ida, end_type) :
        ida(&_ida), offset(_ida.size()) {}

    size_t id() const {
        return ida ? ida->id : not_exists;
    }

    const index_data_accessor &data_accessor() const {
        return *ida;
    }

    uint8_t *pointer() const;
    size_t byte_offset() const;

    private:
    friend class boost::iterator_core_access;

    inline uint8_t &dereference() const;
    inline bool equal(const index_data_iterator &other) const;
    inline void increment();
    inline void decrement();
    inline void advance(std::streamoff n);
    inline std::streamoff distance_to(const index_data_iterator &other) const;

    inline uint8_t *base_address() const { return NULL; }

    private:
    const index_data_accessor *ida;
    size_t offset;
};


// little endian

template<>
inline uint8_t *index_data_iterator<mapped_region>::base_address() const {
    return (uint8_t*) ida->get_address();
}

template<>
inline uint8_t &index_data_iterator<mapped_region>::dereference() const {
    return *(base_address() + offset);
}

template<>
inline uint8_t *index_data_iterator<mapped_region>::pointer() const {
    return base_address() + offset;
}

template<>
inline size_t index_data_iterator<mapped_region>::byte_offset() const {
    return offset;
}

template<>
inline bool index_data_iterator<mapped_region>::equal(const index_data_iterator &other) const {
    return offset == other.offset && base_address() == other.base_address();
}

template<>
inline void index_data_iterator<mapped_region>::increment() { assert(++offset > 0); }

template<>
inline void index_data_iterator<mapped_region>::decrement() { assert(offset-- > 0); }

template<>
inline void index_data_iterator<mapped_region>::advance(std::streamoff n) { offset += n; }

template<>
inline std::streamoff index_data_iterator<mapped_region>::distance_to(
        const index_data_iterator &other) const {
    assert(base_address() == other.base_address());
    return other.offset - offset;
}


// big endian

template<>
inline uint8_t *index_data_iterator<be_mapped_region>::base_address() const {
    return (uint8_t*) ida->get_address();
}

template<>
inline uint8_t &index_data_iterator<be_mapped_region>::dereference() const {
    return *(base_address() + ((offset | 7) - (offset & 7)));
}

template<>
inline uint8_t *index_data_iterator<be_mapped_region>::pointer() const {
    return base_address() + ((offset | 7) - (offset & 7));
}

template<>
inline size_t index_data_iterator<be_mapped_region>::byte_offset() const {
    return offset;
}

template<>
inline bool index_data_iterator<be_mapped_region>::equal(const index_data_iterator &other) const {
    return offset == other.offset && base_address() == other.base_address();
}

template<>
inline void index_data_iterator<be_mapped_region>::increment() { assert(++offset > 0); }

template<>
inline void index_data_iterator<be_mapped_region>::decrement() { assert(offset-- > 0); }

template<>
inline void index_data_iterator<be_mapped_region>::advance(std::streamoff n) { offset += n; }

template<>
inline std::streamoff index_data_iterator<be_mapped_region>::distance_to(
        const index_data_iterator &other) const {
    assert(base_address() == other.base_address());
    return other.offset - offset;
}

}

#endif
