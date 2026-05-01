#ifndef CIRCULAR_DOUBLE_LINKED_LIST_H
#define CIRCULAR_DOUBLE_LINKED_LIST_H

#include "doublelinkedlist.h"
#include <mutex>
#include <shared_mutex>
#include <iostream>
#include "CircularLinkedList.h"

#include "../types.h"

template <typename T>
struct AscendingCDLLTrait : BaseTrait<DLLNode<T>, less<T>>{
};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<DLLNode<T>, greater<T>>{
};



template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = CircularDoubleLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;
    using const_iterator = CLLForwardIterator<const CircularDoubleLinkedList<Trait>>;
    friend const_iterator;

    //iteradores
    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }
    const_iterator begin() const { return const_iterator(this, this->m_pRoot); }
    const_iterator end()   const { return const_iterator(this, nullptr); }


public:

    friend std::ostream& operator<<(std::ostream& os, const CircularDoubleLinkedList<Trait>& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        print_list(os, list);
        os << "]";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, CircularDoubleLinkedList& list) {
        std::unique_lock<std::shared_mutex> lock(list.m_mtx);
        return read_list(is, list);
    }


    void insert(const value_type &value, Ref ref) override {

        // romper si existe tail
        if (this->m_tail) {
            this->m_tail->setNext(nullptr);
        }

        DoubleLinkedList<Trait>::insert(value, ref);

        // restaurar si hay elementos
        if (this->m_tail) {
            this->m_tail->setNext(this->m_pRoot);
        }
    }


    //destructor seguro
    virtual ~CircularDoubleLinkedList() {
        clear();
    }
    void clear() {

        if (this->m_tail) {
            this->m_tail->setNext(nullptr); //romper ciclo
        }

        DoubleLinkedList<Trait>::clear();   // reutilizas base
    }

};




#endif
