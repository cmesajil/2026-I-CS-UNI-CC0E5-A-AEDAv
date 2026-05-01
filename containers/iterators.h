#include "general_iterator.h"

template <typename Container>
class ForwardIterator
    : public general_iterator<Container, ForwardIterator<Container>> {

public:
    using MySelf = ForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf& operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getNext(); // 🔥 clave
        }
        return *this;
    }
};
