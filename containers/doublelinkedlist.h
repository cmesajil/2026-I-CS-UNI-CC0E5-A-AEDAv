

#include "linkedlist.h"


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



// TODO Los iteradores ahora son forward y backward
// Crear 2 nuevos i
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
    // 1. Necesitas exponer los tipos del Trait para que el compilador los encuentre
    using Node = typename Trait::Node;
    using value_type = typename Trait::value_type;

private:
    // Helper: Versión adaptada para DLL
    void internal_insert(Node* &current, Node* prevNode, const value_type &value, Ref ref) {
        // Caso base: hemos llegado al final o al punto donde el valor es menor/mayor
        if (!current || this->m_comp(value, current->getDataRef())) {

            // 1. Crear nuevo nodo: next = current, prev = prevNode
            Node* newNode = new Node(value, ref, current, prevNode);
            // --- DEBUG ---
            std::cout << "Insertando: " << value;
            if (prevNode) std::cout << " (Prev: " << prevNode->getData() << ")";
            else std::cout << " (Prev: NULL)";
            if (current) std::cout << " (Next: " << current->getData() << ")";
            else std::cout << " (Next: NULL)";
            std::cout << std::endl;
            // --------------
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

    // Sobreescribimos el insert principal
    void insert(const value_type &value, Ref ref) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);

        // Llamamos a la versión DLL, comenzando con prevNode = nullptr
        internal_insert(this->m_pRoot, nullptr, value, ref);

        // El m_tail ya se actualiza dentro de internal_insert,
        // no necesitamos el bucle manual de la clase base.
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

    void printBackward() {
        std::shared_lock<std::shared_mutex> lock(this->m_mtx);
        Node* current = this->m_tail;
        std::cout << "Recorrido inverso: ";
        while (current != nullptr) {
            std::cout << current->getData() << " ";
            current = current->getPrev(); // Aquí está la prueba de fuego
        }
        std::cout << std::endl;
    }
    // TODO: Copy constructor
    //       Simplificar y abstraer el bucle de copia de Nodes
    //       Es posible que no necesites este constructor ya que lo heredaste

    // TODO: Move constructor
    // TODO: Copy assignment operator
    // TODO: Move assignment operator

    virtual void verifyLinks();

};

template <typename Trait>
void DoubleLinkedList<Trait>::verifyLinks() {
    std::shared_lock<std::shared_mutex> lock(this->m_mtx);
    Node* current = this->m_pRoot;

    while (current != nullptr && current->getNext() != nullptr) {
        Node* nextNode = current->getNext();

        // Verificación crítica: ¿El prev del siguiente es el actual?
        if (nextNode->getPrev() != current) {
            std::cerr << "¡ERROR ESTRUCTURAL! El enlace es inconsistente en el valor: "
            << current->getData() << std::endl;
            return;
        }
        current = nextNode;
    }
    std::cout << "Estructura doblemente enlazada verificada: OK" << std::endl;
}
