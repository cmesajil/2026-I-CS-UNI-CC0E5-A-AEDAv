#ifndef DOUBLELINKEDLIST_H
#define DOUBLELINKEDLIST_H

#include "linkedlist.h"
// DONE Los iteradores ahora son forward y backward

template <typename Container>
class DoubleLinkedList_forward_iterator : public general_iterator<Container, DoubleLinkedList_forward_iterator<Container>> {
public:
    using MySelf = DoubleLinkedList_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf& operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext();
        }
        return *this;
    }
};

template <typename Container>
class DoubleLinkedList_backward_iterator : public general_iterator<Container, DoubleLinkedList_backward_iterator<Container>> {
public:
    using MySelf = DoubleLinkedList_backward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf& operator++() {
        if (this->m_pNode) {
            // Avanzar en el iterador significa retroceder en la lista
            this->m_pNode = this->m_pNode->getPrev();
        }
        return *this;
    }
};



template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    using Base = LLNode<T, DLLNode<T>>;
    using Node = typename Base::Node;
    private:
        Node *m_pPrev;
    public:
        DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
        DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr)
            : Base(data, ref, next), m_pPrev(prev) {}

        Node*  getPrev() const     { return m_pPrev; }
        void   setPrev(Node *prev) { m_pPrev = prev; }
        Node*& getPrevRef()        { return m_pPrev; }

};

template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>>{
    using Node = DLLNode<T>;
};

template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>>{
    using Node = DLLNode<T>;
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{

public:

    using Node = typename Trait::Node;
    using value_type = typename Trait::value_type;
    using  forward_iterator   = DoubleLinkedList_forward_iterator < DoubleLinkedList<Trait> > ;
    friend forward_iterator;
    using  backward_iterator  = DoubleLinkedList_backward_iterator< DoubleLinkedList<Trait> > ;
    friend backward_iterator;

private:

    void internal_insert(Node* &current, Node* prevNode, const value_type &value, Ref ref) {
        // Caso base: hemos llegado al final o al punto donde el valor es menor/mayor
        if (!current || this->m_comp(value, current->getDataRef())) {

            // 1. Crear nuevo nodo: next = current, prev = prevNode
            Node* newNode = new Node(value, ref, current, prevNode);

            // 2. Si el nodo actual existe, su 'prev' debe apuntar al nuevo
            if (current != nullptr) {
                current->setPrev(newNode);
            }

            // 3. 'current' es la referencia al puntero del nodo anterior (o la cabeza)
            // Esto actualiza el 'next' del nodo anterior para que apunte al nuevo
            current = newNode;

            this->m_size++;

            // 4. Actualizar m_tail si es necesario
            if (newNode->getNext() == nullptr) {
                this->m_tail = newNode;
            }
            return;
        }

        // Recursión: pasamos 'current' como el nuevo 'prevNode'
        internal_insert(current->getNextRef(), current, value, ref);
    }

public:

    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    backward_iterator rbegin() { return backward_iterator(this,this->m_tail); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    // Done: Agregar control concurrente
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);

        // El bucle de rango nativo utiliza begin() y end() automáticamente
        for (auto& item : *this) {
            func(item, std::forward<Args>(args)...);
        }
    }

    // Done: Agregar control concurrente

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&... args) {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);

        if (this->m_size == 0) return;

        // Usamos el bucle manual con los iteradores inversos
        for (auto it = rbegin(); it != rend(); ++it) {
            // *it desreferencia el iterador para obtener el dato
            func(*it, std::forward<Args>(args)...);
        }
    }

    // Sobreescribimos el insert principal
    void insert(const value_type &value, Ref ref) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);

        // Llamamos a la versión DLL, comenzando con prevNode = nullptr
        internal_insert(this->m_pRoot, nullptr, value, ref);

        // El m_tail ya se actualiza dentro de internal_insert,
        // no necesitamos el bucle manual de la clase base.
    }


    DoubleLinkedList() : LinkedList<Trait>() {}
    // DONE : Copy constructor
    //       Simplificar y abstraer el bucle de copia de Nodes
    //       Es posible que no necesites este constructor ya que lo heredaste
    DoubleLinkedList(const DoubleLinkedList &other): LinkedList<Trait>(){
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
            push_back(c->getData(), c->getRef());
    }
    // DONE: Move constructor

    DoubleLinkedList(DoubleLinkedList &&other): LinkedList<Trait>(){
        unique_lock<shared_mutex> lock(other.m_mtx);
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    // DONE: Copy assignment operator
    DoubleLinkedList &operator=(const DoubleLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
                push_back(c->getData(), c->getRef());
        }
        return *this;
    }
    // DONE: Move assignment operator
    DoubleLinkedList &operator=(DoubleLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }


    //push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref, nullptr, this->m_tail);
        if (this->m_tail) this->m_tail->setNext(newNode);
        else        this->m_pRoot = newNode;
        this->m_tail = newNode;
        this->m_size++;
    }



    virtual ~DoubleLinkedList() { clear(); }
    void clear(){
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *act = this->m_pRoot;
        while (act) {
            Node *next = act->getNext();
            delete act;
            act = next;
        }
        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }


    // Operadores I/O
    friend istream& operator>>(istream& is, DoubleLinkedList& list) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')') {
                        list.insert(val, ref);
                    }
                }
            }
        }
        return is;
    }
    friend ostream& operator<<(ostream& os, const DoubleLinkedList<Trait>& list) {
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        for (Node* curr = list.m_pRoot; curr != nullptr; curr = curr->getNext()) {
            os << "(" << curr->getData() << "," << curr->getRef() << ")";
            if (curr->getNext()) os << ",";
        }
        os << "]";
        return os;
    }



};



#endif
