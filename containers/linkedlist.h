#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include "../types.h"
#include "general_iterator.h"
#include "util.h"
#include <cstddef> // size_t
#include <iostream>
#include <mutex>
#include <shared_mutex> // shared_mutex
#include <sstream>
#include <string>
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator
    : public general_iterator<Container, LinkedListForwardIterator<Container>> {

public:
  using MySelf = LinkedListForwardIterator<Container>;
  using Parent = general_iterator<Container, MySelf>;
  using Parent::Parent;
  // TODO: Completar el operator++
  MySelf &operator++() {
    this->m_pNode = this->m_pNode->getNext();
    return *this;
  }
};

// Linked List Node
template <typename T> class LLNode {
  using Node = LLNode<T>;

private:
  T m_data;
  Ref m_ref;
  Node *m_next;

public:
  LLNode() : m_data(T()), m_next(nullptr) {}
  LLNode(T data) : m_data(data), m_next(nullptr) {}
  LLNode(T data, Node *next) : m_data(data), m_next(next) {}
  LLNode(T data, Ref ref, Node *next)
      : m_data(data), m_ref(ref), m_next(next) {}
  virtual ~LLNode() {}

  T getData() const { return m_data; }
  T &getDataRef() { return m_data; }
  Ref &getRef() { return m_ref; }
  const Ref &getRef() const { return m_ref; }
  // Versión para objetos constantes (solo lectura)
  const T &getDataRef() const { return m_data; }
  void setData(T data) { m_data = data; }
  Node *getNext() const { return m_next; }
  Node *&getNextRef() { return m_next; }
  void setNext(Node *next) { m_next = next; }

  // DONE operator<<
  friend ostream &operator<<(ostream &os, const LLNode<T> &node) {
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
  }
};

template <typename T> struct AscendingLinkedListTrait {
  using value_type = T;
  using Node = LLNode<T>;
  using Comp = less<T>;
};

template <typename T> struct DescendingLinkedListTrait {
  using value_type = T;
  using Node = LLNode<T>;
  using Comp = greater<T>;
};

template <typename Trait> class LinkedList {
public:
  using value_type = typename Trait::value_type;
  using Node = typename Trait::Node;
  using Comp = typename Trait::Comp;
  using MySelf = LinkedList<Trait>;

  using forward_iterator = LinkedListForwardIterator<MySelf>;
  // friend forward_iterator;

private:
  Node *m_pRoot = nullptr;
  Node *m_tail = nullptr;
  size_t m_size = 0;
  Comp m_comp;
  mutable shared_mutex m_mtx;

public:
  LinkedList() {}

  // DONE: Copy constructor
  LinkedList(const LinkedList &other)
      : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
    Node *current = other.m_pRoot;
    while (current != nullptr) {
      this->push_back(current->getData(), current->getRef());
      current = current->getNext();
    }
  }

  // DONE: Constructor de Movimiento
  LinkedList(LinkedList &&other) noexcept
      : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
    // "Robamos" los recursos de other
    swap(*this, other);
  }

  LinkedList &operator=(const LinkedList &other) { // Copy assignment operator
  }

  LinkedList &operator=(LinkedList &&other) { // Move assignment operator
  }

  // Función amigo para el intercambio (Swap)
  friend void swap(LinkedList &first, LinkedList &second) noexcept {
    std::swap(first.m_pRoot, second.m_pRoot);
    std::swap(first.m_tail, second.m_tail);
    std::swap(first.m_size, second.m_size);
  }

  virtual ~LinkedList();
  virtual void push_front(value_type value, Ref ref);
  virtual void pop_front();
  virtual void push_back(value_type value, Ref ref);
  virtual void pop_back();

private:
  void internal_insert(Node *&pParent, const value_type &value, Ref ref);

public:
  virtual void insert(const value_type &value, Ref ref);

  // virtual value_type& operator[](size_t index);
  // virtual size_t  size() const;
  // DONE operator<<
  string toString() const {
    Node *current = m_pRoot;
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < m_size; ++i) {
      if (i > 0)
        oss << ",";
      oss << *current;
      current = current->getNextRef();
    }
    oss << "]";
    return oss.str();
  }

  forward_iterator begin() { return forward_iterator(this, m_pRoot); }
  forward_iterator end() { return forward_iterator(this, nullptr); }

  // Agregar Foreach
  template <typename Func, typename... Args>
  void ForEach(Func func, Args &&...args) {
    unique_lock<shared_mutex> lock(m_mtx);
    ::ForEach(begin(), end(), func, std::forward<Args>(args)...);
  }

  // DONE:operador[]
  value_type &operator[](Ref index) {
    Node *iter = m_pRoot;
    while (iter != nullptr) {
      if (iter->getRef() == index)
        return iter->getDataRef();
      iter = iter->getNext();
    }

    throw std::out_of_range("Ref not found");
  }
};

// DONE:pop_front
template <typename T> void LinkedList<T>::pop_front() {
  if (m_pRoot == nullptr)
    return;
  Node *root_temp = m_pRoot;
  m_pRoot = m_pRoot->getNext();
  root_temp->setNext(nullptr);
  if (m_pRoot == nullptr) {
    m_tail = nullptr;
  }
  delete root_temp;
  m_size--;
}

// DONE:pop_back
template <typename T> void LinkedList<T>::pop_back() {
  if (m_pRoot == nullptr)
    return;
  // Caso lista con 1 solo elemento
  if (m_pRoot == m_tail) {
    delete m_pRoot;
    m_pRoot = nullptr;
    m_tail = nullptr;
    m_size--;
    return;
  }

  Node *pretail = m_pRoot;
  while (pretail->getNextRef() != m_tail) {
    pretail = pretail->getNextRef();
  }
  delete m_tail;
  m_tail = pretail;
  m_tail->setNext(nullptr);
  m_size--;
}

// DONE:push_front
template <typename T>
void LinkedList<T>::push_front(value_type value, Ref ref) {
  Node *root = new Node(value, ref, m_pRoot);
  m_pRoot = root;
  if (m_tail == nullptr) {
    m_tail = root;
  }
  m_size++;
}

// DONE:push_back
template <typename T> void LinkedList<T>::push_back(value_type value, Ref ref) {
  Node *tail = new Node(value, ref, nullptr);
  if (m_pRoot == nullptr) {
    m_pRoot = tail;
    m_tail = tail;
  } else {
    m_tail->setNext(tail);
    m_tail = tail;
  }
  m_size++;
}

// arreglando la actualizacion tail
template <typename T>
void LinkedList<T>::internal_insert(Node *&pPrev, const value_type &value,
                                    Ref ref) {
  if (!pPrev || m_comp(value, pPrev->getDataRef())) {
    pPrev = new Node(value, ref, pPrev);
    m_size++;
    if (pPrev->getNext() == nullptr) {
      m_tail = pPrev;
    }
    return;
  }
  internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename T>
void LinkedList<T>::insert(const value_type &value, Ref ref) {
  internal_insert(m_pRoot, value, ref);
}

// DONE operator<<
template <typename T>
ostream &operator<<(ostream &os, const LinkedList<T> &list) {
  return os << list.toString();
}

// DONE operator<<
template <typename T> istream &operator>>(istream &is, LinkedList<T> &list) {
  char ch;
  is >> ch; // lee '['
  while (true) {
    is >> ch; // '(' o ']'

    if (ch == ']')
      break;

    typename LinkedList<T>::value_type value;
    Ref ref;

    is >> value;
    is >> ch;
    is >> ref;
    is >> ch;

    list.insert(value, ref);

    is >> ch; // ',' o ']'
    if (ch == ']')
      break;
  }

  return is;
}

template <typename T> LinkedList<T>::~LinkedList() {
  Node *current = m_pRoot;

  while (current != nullptr) {
    // 1. Guardamos el siguiente nodo antes de borrar el actual
    Node *nextNode = current->getNext();

    // 2. Borramos el nodo actual
    delete current;

    // 3. Saltamos al siguiente nodo que guardamos
    current = nextNode;
  }

  m_pRoot = nullptr;
  m_tail = nullptr;
  m_size = 0;
}

void LinkedListDemo();
void ListsDemo();
#endif // __LINKEDLIST_H__
