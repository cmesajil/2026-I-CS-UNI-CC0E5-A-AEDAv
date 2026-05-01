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

private:
    // Internal insert (debe ser privado para que nadie toque los punteros directamente)
    void internal_insert(const value_type &value, Ref ref) {
        Node *newNode = new Node(value, ref);
        this->m_size++;

        // Caso 1: Lista vacía
        if (!this->m_pRoot) {
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
            this->m_pRoot = newNode;
            this->m_tail = newNode;
            return;
        }

        // Caso 2: Nuevo valor va antes del root
        if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            Node *last = this->m_tail;
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(last);
            this->m_pRoot->setPrev(newNode);
            last->setNext(newNode);
            this->m_pRoot = newNode;
            return;
        }

        // Caso general
        Node *act = this->m_pRoot;
        while (act->getNext() != this->m_pRoot && !this->m_comp(value, act->getNext()->getDataRef())) {
            act = act->getNext();
        }

        Node *next_node = act->getNext();
        newNode->setNext(next_node);
        newNode->setPrev(act);
        act->setNext(newNode);
        next_node->setPrev(newNode);

        if (act == this->m_tail) {
            this->m_tail = newNode;
        }
    }

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
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        internal_insert(value, ref);
    }


    //destructor seguro
    virtual ~CircularDoubleLinkedList() {
        clear();
    }
    void clear() {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) return;

        Node* act = this->m_pRoot;
        Node* next = nullptr;

        // Rompemos la circularidad para poder iterar tranquilamente
        this->m_tail->setNext(nullptr);

        while (act != nullptr) {
            next = act->getNext();
            delete act;
            act = next;
        }

        this->m_pRoot = nullptr;
        this->m_tail = nullptr;
        this->m_size = 0;
    }

};




#endif
