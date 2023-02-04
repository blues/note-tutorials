//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once

// library code

#include <memory>
#include <Notecard.h>

extern Notecard notecard;


template <typename T, typename D=std::default_delete<T>>
class unique_c_ptr : public std::unique_ptr<T, D> {
    using Pointer = T*;
    using super = std::unique_ptr<T, D>;
public:
    unique_c_ptr(T* t, D d) noexcept : super(t, d) {}

    operator Pointer() const {
        return this->get();
    }
};

typedef unique_c_ptr<uint8_t, void(*)(void*)> scoped_malloc_t;

inline scoped_malloc_t scoped_malloc() {
    return scoped_malloc_t{nullptr, free};
}

inline scoped_malloc_t scoped_malloc(size_t size) {
    return scoped_malloc_t{reinterpret_cast<uint8_t*>(malloc(size)), free};
}

/**
 * @brief Explicitly free the memory previously allocated. This is typically not required
 * as the memory is freed when the variable goes out of scope.
 * 
 * @param scope 
 */
inline void scoped_free(scoped_malloc_t& scope) {
    scope.release();
}

typedef unique_c_ptr<J, void(*)(J*)> scoped_response_t;

/**
 * @brief Create a handle for a response so that it is freed when it goes out of scope.
 * 
 * @param rsp
 * @return auto 
 */
inline scoped_response_t scoped_response(J* rsp) {
    // ideally would use notecard.deleteResponse or NoteDeleteResponse, but the former is a non-static method
    // and the latter is a macro that calls JDelete
    return scoped_response_t{rsp, JDelete};
}


inline int JB64Decode(unsigned char *bufplain, const char *bufcoded) {
    return JB64Decode((char*)bufplain, bufcoded);
}