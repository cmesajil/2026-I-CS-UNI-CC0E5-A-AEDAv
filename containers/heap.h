#ifndef __HEAP_H__
#define __HEAP_H__

#include <iostream>
#include <cstddef> // size_t
#include <iterator>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <tuple>
#include "util.h"
#include "../types.h"
#include "traits.h"
#include "vector.h"

using namespace std;

template <typename T>
struct MinHeapTrait : public BaseTrait<VectorNode<T>, less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseTrait<VectorNode<T>, greater<T>> {};

template<typename T,typename _Comp>
class HeapNode{
public:
    using value_type = T;
    using MySelf     = HeapNode<T,_Comp>;
    using Comp       = _Comp;

private:
    value_type m_data;
    Ref        m_ref;
    Comp       m_comp;
public:
    HeapNode(value_type data, Ref ref, Comp comp) : m_data(data), m_ref(ref), m_comp(comp) {}
    ~HeapNode() {}
    value_type getData() const { return m_data; }
    Ref getRef() const { return m_ref; }

    // Compara este nodo con otro delegando la lógica a m_comp
    bool operator<(const HeapNode& other) const {
        return m_comp(m_data, other.m_data);
    }
};

template<typename Trait>
class Heap{
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;
    using Node       = typename Trait::Node;

private:
    Vector<Trait> m_vec;
    Comp          m_comp;
    mutable shared_mutex m_mtx;
public:
    Heap() : m_vec(), m_comp() {}
    Heap(size_t capacity) : m_vec(capacity), m_comp() {}
    ~Heap() {}
    Vector<Trait>* getVector() { return &m_vec; }
    bool isEmpty(){
        return m_vec.empty();
    }
    size_t size(){
        return m_vec.size();
    }
    string toString(){
        return m_vec.toString();
    }

    void heapifyUp(size_t index){
        if (index == 0) return; // Caso base implícito de forma segura

        size_t parent = (index - 1) / 2;
        if (m_comp(m_vec[index].getData(), m_vec[parent].getData())){
            // std::swap usa los operadores de movimiento de tu VectorNode de forma segura
            std::swap(m_vec[index], m_vec[parent]);
            heapifyUp(parent);
        }
    }
    void heapifyDown(size_t index){
        size_t left    = 2 * index + 1;
        size_t right   = 2 * index + 2;
        size_t smallest = index;

        if (left < m_vec.size() && m_comp(m_vec[left].getData(), m_vec[smallest].getData())){
            smallest = left;
        }
        if (right < m_vec.size() && m_comp(m_vec[right].getData(), m_vec[smallest].getData())){
            smallest = right;
        }
        if (smallest != index){
            // Intercambio seguro en lugar de sobreescritura destructiva
            std::swap(m_vec[index], m_vec[smallest]);
            heapifyDown(smallest);
        }
    }

    void insert(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);
        m_vec.push_back(value,ref);
        heapifyUp(m_vec.size() - 1);
    }

    // Extrae el elemento de mayor o menor prioridad (depende del heap)
    void extract() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.empty()) return;

        // Usamos el operador de asignación por movimiento que ya definiste en VectorNode
        m_vec[0] = std::move(m_vec[m_vec.size() - 1]);

        // Reducimos el tamaño del vector de forma segura
        m_vec.pop_back();

        // Reestructuramos el Heap hacia abajo
        heapifyDown(0);
    }

    // Obtiene el elemento de mayor o menor prioridad (depende del heap)
    // sin removerlo
    Node peek(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_vec.empty()) return Node(value_type(), Ref());
        return m_vec[0];
    }

};

void DemoHeap();


#endif // __HEAP_H__
