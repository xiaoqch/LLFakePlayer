#pragma once
template <__int64 hash>
inline void* _dlsym_static(const char* sym)
{
    static void* ptr = dlsym_real(sym);
    return ptr;
};

// Cacheable dlsym
#define dlsym_static(sym) _dlsym_static<do_hash(sym)>(sym)

#define SymCallStatic(fn, ret, ...) ((ret(*)(__VA_ARGS__))(dlsym_static(fn)))