#ifndef __TRAITS_H__
#define __TRAITS_H__

template <typename _Node, typename _Comp>
struct BaseTrait{
    using value_type = typename _Node::value_type;
    using Node = _Node;
    using Comp = _Comp;
};

#endif // __TRAITS_H__
