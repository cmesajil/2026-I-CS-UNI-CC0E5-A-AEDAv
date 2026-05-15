#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <cstdint> // Necesario para intptr_t o uintptr_t

// C++11, C++14, C++17, C++20, C++23 ...
using Type = int;

// DETECCIÓN AUTOMÁTICA DE ARQUITECTURA (32-bit vs 64-bit)
// Evaluamos el tamaño de un puntero en la arquitectura destino.
// Si los punteros miden 8 bytes (64 bits), usamos long long. Si no, int.
#if UINTPTR_MAX == 0xffffffffffffffffULL
    using T1 = long long; // 64-bit architecture
#else
    using T1 = int;       // 32-bit architecture
#endif

using T2  = std::string;
using Ref = long;

#endif // __TYPES_H__
