#pragma once
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
            this->m_pNode = this->m_pNode->getNext();
        }
        return *this;
    }
};

template <typename Container>
class BackwardIterator
    : public general_iterator<Container, BackwardIterator<Container>> {

public:
    using MySelf = BackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;

    MySelf& operator++() {
        if (this->m_pNode) {
            this->m_pNode = this->m_pNode->getPrev();
        }
        return *this;
    }
};
