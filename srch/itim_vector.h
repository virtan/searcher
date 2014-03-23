#ifndef ITIM_VECTOR_H
#define ITIM_VECTOR_H

#include <vector>

namespace itim {

/*
    template<class value_type>
    class fast_vector : public std::vector<value_type>
    {
        public:
        fast_vector() {}
        fast_vector(const fast_vector &other) : std::vector<value_type>(other) {}
    };
*/
    template<class value_type>
    class fast_vector : public std::vector<value_type>
    {
        private:
        typedef std::vector<value_type> stdvect;

        public:
        fast_vector() : stdvect(), real_size(0) {}
        fast_vector(const stdvect &other) {
            operator=(other);
        }

        void clear() { real_size = 0; }
        size_t size() { return real_size; }
        void reserve(size_t ns) { if(this->stdvect::size() < ns) this->stdvect::reserve(ns); }
        void push_back(const value_type &v) {
            if(this->stdvect::size() > real_size) {
                this->stdvect::operator[](real_size++) = v;
            } else {
                this->stdvect::push_back(v);
                ++real_size;
            }
        }
        value_type &back() {
            return this->operator[](real_size - 1);
        }
        stdvect &operator=(const stdvect &other) {
            real_size = 0;
            for(auto i = other.begin(); i != other.end(); ++i)
                push_back(*i);
        }

        private:
        size_t real_size;
    };

}

#endif
