/*++
Copyright (c) 2006 Microsoft Corporation

Module Name:

    array.h

Abstract:

    Fixed size arrays

Author:

    Leonardo de Moura (leonardo) 2011-01-26.

Revision History:

--*/
#ifndef __ARRAY_H_
#define __ARRAY_H_

template<typename T, bool CallDestructors=true>
class array {
public:
    // Return the space needed to store an array of size sz.
    static size_t space(size_t sz) { return sizeof(T)*sz + sizeof(size_t); }
    
private:
#define ARRAY_SIZE_IDX     -1
    T * m_data;
    void destroy_elements() {
        iterator it = begin();
        iterator e  = end();
        for (; it != e; ++it) {
            it->~T();
        }
    }

    char * raw_ptr() const { return reinterpret_cast<char*>(reinterpret_cast<size_t*>(m_data) - 1); }

    array & operator=(array const & source);

    void set_data(void * mem, size_t sz) {
        size_t * _mem = static_cast<size_t*>(mem);
        *_mem = sz; 
        _mem ++;
        m_data = reinterpret_cast<T*>(_mem);
    }

    template<typename Allocator>
    void allocate(Allocator & a, size_t sz) {
        size_t * mem  = reinterpret_cast<size_t*>(a.allocate(space(sz)));
        set_data(mem, sz);
    }

    void init() {
        iterator it = begin();
        iterator e  = end();
        for (; it != e; ++it) {
            new (it) T();
        }
    }
    
    void init(T const * vs) {
        iterator it = begin();
        iterator e  = end();
        for (; it != e; ++it, ++vs) {
            new (it) T(*vs); 
        }
    }

public:
    typedef T data;
    typedef T * iterator;
    typedef const T * const_iterator;

    array():m_data(0) {}

    /**
       \brief Store the array in the given chunk of memory (mem).
       This chunck should be big enough to store space(sz) bytes.
    */
    array(void * mem, size_t sz, T const * vs) {
        DEBUG_CODE(m_data = 0;);
        set(mem, sz, vs);
    }

    // WARNING: the memory allocated will not be automatically freed.
    array(void * mem, size_t sz, bool init_mem) {
        DEBUG_CODE(m_data = 0;);
        set_data(mem, sz);
        if (init_mem)
            init();
    }

    // WARNING: the memory allocated will not be automatically freed.
    template<typename Allocator>
    array(Allocator & a, size_t sz, T const * vs) {
        DEBUG_CODE(m_data = 0;);
        set(a, sz, vs);
    }

    // WARNING: the memory allocated will not be automatically freed.
    template<typename Allocator>
    array(Allocator & a, size_t sz, bool init_mem) {
        DEBUG_CODE(m_data = 0;);
        allocate(a, sz);
        if (init_mem)
            init();
    }
    
    // WARNING: this does not free the memory used to store the array.
    // You must free it yourself, or use finalize.
    ~array() {
        if (m_data && CallDestructors)
            destroy_elements();
    }

    // Free the memory used to store the array.
    template<typename Allocator>
    void finalize(Allocator & a) {
        if (m_data) {
            if (CallDestructors)
                destroy_elements();
            a.deallocate(size(), raw_ptr());
            m_data = 0;
        }
    }

    void set(void * mem, size_t sz, T const * vs) {
        SASSERT(m_data == 0);
        set_data(mem, sz);
        init(vs);
    }
    
    template<typename Allocator>
    void set(Allocator & a, size_t sz, T const * vs) {
        SASSERT(m_data == 0);
        allocate(a, sz);
        init(vs);
    }

    size_t size() const { 
        if (m_data == 0) {
            return 0;  
        }
        return reinterpret_cast<size_t *>(m_data)[SIZE_IDX]; 
    }
    
    bool empty() const { return m_data == 0; }

    T & operator[](size_t idx) { 
        SASSERT(idx < size()); 
        return m_data[idx]; 
    }

    T const & operator[](size_t idx) const { 
        SASSERT(idx < size()); 
        return m_data[idx];
    }

    iterator begin() { 
        return m_data; 
    }

    iterator end() { 
        return m_data + size();
    }

    const_iterator begin() const { 
        return m_data; 
    }

    const_iterator end() const { 
        return m_data + size(); 
    }

    T * c_ptr() { return m_data; }
};

template<typename T>
class ptr_array : public array<T *, false> {
public:
    ptr_array() {}
    ptr_array(void * mem, size_t sz, T * const * vs):array<T*, false>(mem, sz, vs) {}
    template<typename Allocator>
    ptr_array(Allocator & a, size_t sz, T * const * vs):array<T*, false>(a, sz, vs) {}
    ptr_array(void * mem, size_t sz, bool init_mem):array<T*, false>(mem, sz, init_mem) {}
    template<typename Allocator>
    ptr_array(Allocator & a, size_t sz, bool init_mem):array<T*, false>(a, sz, init_mem) {}
};

template<typename T>
class sarray : public array<T, false> {
public:
    sarray() {}
    sarray(void * mem, size_t sz, T const * vs):array<T, false>(mem, sz, vs) {}
    template<typename Allocator>
    sarray(Allocator & a, size_t sz, T const * vs):array<T, false>(a, sz, vs) {}
    sarray(void * mem, size_t sz, bool init_mem):array<T, false>(mem, sz, init_mem) {}
    template<typename Allocator>
    sarray(Allocator & a, size_t sz, bool init_mem):array<T, false>(a, sz, init_mem) {}
};

#endif
