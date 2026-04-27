#ifndef CIRCULAR_DOUBLE_LINKED_LIST_H
#define CIRCULAR_DOUBLE_LINKED_LIST_H

#include "doublelinkedlist.h"
#include <mutex>
#include <shared_mutex>
#include <iostream>

// NOTA: Mueve tus structs de Traits a un archivo llamado "CircularTraits.h"
// para evitar el error de redefinición que te dio el compilador.

template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>>{
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>>{
    using Node = DLLNode<T>;
};



template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = CircularDoubleLinkedList<Trait>;

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
        if (list.m_pRoot) {
            Node *act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if (act != list.m_pRoot) os << ",";
            } while (act != list.m_pRoot);
        }
        os << "]";

        if (list.m_pRoot)
            os << " ->root(" << list.m_pRoot->getData()<< "," << list.m_pRoot->getRef() << ")";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, CircularDoubleLinkedList<Trait>& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(std::ios_base::failbit);
            return is;
        }

        typename Trait::value_type val;

        int ref_temporal;

        char comma, parenClose;

        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref_temporal >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        list.insert(val, ref_temporal);
                    }
                }
            }
        }
        return is;
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
