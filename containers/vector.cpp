#include <cstddef>
#include <iostream>
#include "vector.h"

using namespace std;

Vector::Vector(size_t capacity){
    m_capacity = capacity;
    m_size = 0;
    m_data = new T1[capacity];
}

Vector::~Vector(){
    delete[] m_data;
}

void Vector::push_back(T1 value){
    if(m_size < m_capacity)
        m_data[m_size++] = value;
}

T1 Vector::get(size_t index){
    if(index >= 0 && index < m_size)
        return m_data[index];
    return -1;
}

size_t Vector::size(){
    return m_size;
}

void DemoVector(){
    Vector v(10);
    v.push_back(1);
    v.push_back(2);
    v.push_back(-1);
    v.push_back(4);
    for(size_t i = 0; i < v.size(); ++i){
        std::cout << v.get(i) << " ";
    }
    std::cout << std::endl;
}