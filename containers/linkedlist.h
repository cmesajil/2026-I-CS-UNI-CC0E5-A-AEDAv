#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // TODO: Completar el operator++
    // MySelf& operator++(){
        
    // }
};

// Linked List Node
template <typename T, typename NodeType = LLNode<T>>
class LLNode{
protected:
    using Node = NodeType;
private:
    T     m_data;
    Ref   m_ref;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next = nullptr) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const     { return m_data; }
    T&     getDataRef()        { return m_data; }
    void   setData(T data)     { m_data = data; }
    Ref    getRef() const      { return m_ref; }
    
    Node*  getNext() const     { return m_next; }
    void   setNext(Node *next) { m_next = next; }
    Node*& getNextRef()        { return m_next; }
    
};

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    // friend forward_iterator;

private:
    Node  *m_pRoot = nullptr;
    Node  *m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
public:
    LinkedList() {}
    // TODO: Copy constructor
    LinkedList(const LinkedList &other){ }
    // Move constructor
    LinkedList(LinkedList &&other){ }
    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
    }
    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
    }
    
    virtual        ~LinkedList() {}
    virtual void    push_front(value_type value, Ref ref);
    virtual void    pop_front();
    virtual void    push_back(value_type value, Ref ref);
    virtual void    pop_back();
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;
    virtual string  toString() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // Agregar Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

template <typename T>
void LinkedList<T>::internal_insert(Node* &pPr  ev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev == m_pRoot)
            m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename T>
void LinkedList<T>::insert(const value_type &value, Ref ref){
    internal_insert(m_pRoot, value, ref);
}



#endif // __LINKEDLIST_H__