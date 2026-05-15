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
    HeapNode() : m_data(T()), m_ref(Ref()), m_comp(Comp()) {}
    HeapNode(value_type data, Ref ref, Comp comp = Comp()) : m_data(data), m_ref(ref), m_comp(comp) {}
    ~HeapNode() {}
    value_type getData() const { return m_data; }
    Ref getRef() const { return m_ref; }

    // Compara este nodo con otro delegando la lógica a m_comp
    bool operator<(const HeapNode& other) const {
        return m_comp(m_data, other.m_data);
    }

    friend std::ostream& operator<<(std::ostream& os, const HeapNode& node) {
            // Formato limpio: ej. "5 (Ref: 0x7ffd)" o simplemente "5" si prefieres solo el dato
            os << "(" << node.m_data << "," << node.m_ref << ")";;
            return os;
        }
};

// Ahora el Trait del Heap usa HeapNode en lugar de VectorNode
template <typename T>
struct MinHeapTrait : public BaseTrait<HeapNode<T, std::less<T>>, std::less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseTrait<HeapNode<T, std::greater<T>>, std::greater<T>> {};

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
    bool isEmpty() {
        return m_vec.empty();
    }
    size_t size() {
        return m_vec.size();
    }
    string toString() const{
        return m_vec.toString();
    }

    void heapifyUp(size_t index){
        if (index == 0) return; // Caso base implícito de forma segura

        size_t parent = (index - 1) / 2;
        if (m_vec[index] < m_vec[parent]) {
                    std::swap(m_vec[index], m_vec[parent]);
                    heapifyUp(parent);
                }
    }
    void heapifyDown(size_t index){
        size_t left    = 2 * index + 1;
        size_t right   = 2 * index + 2;
        size_t smallest = index;

        // Usamos el operador sobrecargado uniformemente
            if (left < m_vec.size() && m_vec[left] < m_vec[smallest]){
                smallest = left;
            }
            if (right < m_vec.size() && m_vec[right] < m_vec[smallest]){
                smallest = right;
            }
            if (smallest != index){
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

            m_vec[0] = std::move(m_vec[m_vec.size() - 1]);
            m_vec.pop_back();
            heapifyDown(0);
        }

    // Obtiene el elemento de mayor o menor prioridad (depende del heap)
    // sin removerlo
    Node peek() {
            shared_lock<shared_mutex> lock(m_mtx);
            if (m_vec.empty()) return Node(value_type(), Ref());
            return m_vec[0];
        }

    friend std::ostream& operator<<(std::ostream& os, const Heap& heap) {
        std::shared_lock<std::shared_mutex> lock(heap.m_mtx);
        os << heap.toString();
        return os;
    }
    friend std::istream& operator>>(std::istream& is, Heap& heap) {
        //std::unique_lock<std::shared_mutex> lock(heap.m_mtx); ya esta en insert
        return read_list(is, heap);
    }

};

void DemoHeap();


#endif // __HEAP_H__
