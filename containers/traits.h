#ifndef __TRAITS_H__
#define __TRAITS_H__

template <typename T, typename _Comp>
struct BaseTrait{
    using value_type = T;
    using Comp = _Comp;
};

#endif // __TRAITS_H__