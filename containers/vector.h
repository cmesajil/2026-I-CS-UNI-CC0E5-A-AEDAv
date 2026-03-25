#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <cstddef> // size_t
#include "../types.h"

class Vector{
private:
    size_t  m_capacity;
    size_t  m_size;
    T1 * m_data;
public:
    Vector(size_t capacity);
    virtual ~Vector();
    virtual void push_back(T1 value);
    virtual T1  get(size_t index);
    virtual size_t  size();
};

void DemoVector();

#endif // __VECTOR_H__
