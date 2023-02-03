//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once

// library code

#include <memory>
#include <Notecard.h>

extern Notecard notecard;

typedef std::unique_ptr<uint8_t[], void(*)(void*)> scoped_malloc_t;

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

typedef std::unique_ptr<J, void(*)(J*)> scoped_response_t;

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

#if 0 // WiP trials
class JSONWrapper {
    J* j;

public: // want it to be package private
    JSONWrapper(J* json) : j(json) {}

public:
    typedef const char* name;

    auto& add(name n, const char* s) {
        JAddStringToObject(j, n, s);
        return *this;
    }

#ifdef ARDUINO
    auto& add(name n, const String& s) {
        add(n, s.c_str());
        return *this;
    }
#endif

    // auto& add(name n, int i) {
    //     JAddNumberToObject(j, n, i);
    //     return *this;
    // }

    auto& add(name n, double d) {
        JAddNumberToObject(j, n, d);
        return *this;
    }

    auto addObject(name n) {
        J* o = JAddObjectToObject(j, n);
        return JSONWrapper(o);
    }

    operator bool() const {
        return j;
    }

    J* clear() {
        J* old = j;
        j = nullptr;
        return old;
    }
};


class Request : public JSONWrapper {

public:
    Request(const char* name) :
        JSONWrapper(notecard.newRequest(name))
    {
    }
};

class RequestBuilder {
public:

    Request request(const char* name) const {
        return Request(name);
    }


};

static const RequestBuilder Notecard2;
#endif 