/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <memory>

namespace cflib { namespace serialize {

// Forward declarations
class SerializeVariableTypeInfo;
class SerializeFunctionTypeInfo;

// We use a thin wrapper around std::vector to handle forward-declared types.
// This works because the wrapper itself does not instantiate vector operations in the header.
template<typename T>
class TypeInfoList
{
public:
    TypeInfoList();
    ~TypeInfoList();
    TypeInfoList(const TypeInfoList &);
    TypeInfoList(TypeInfoList &&) noexcept;
    TypeInfoList & operator=(const TypeInfoList &);
    TypeInfoList & operator=(TypeInfoList &&) noexcept;

    void push_back(const T & val);
    void push_back(T && val);
    void clear();
    bool empty() const;
    cfsize_t size() const;
    T & operator[](cfsize_t i);
    const T & operator[](cfsize_t i) const;

    // Iterator support
    using iterator = T *;
    using const_iterator = const T *;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// operator<< for TypeInfoList
template<typename T>
inline TypeInfoList<T> & operator<<(TypeInfoList<T> & list, const T & val) {
    list.push_back(val);
    return list;
}

class SerializeTypeInfo
{
public:
    enum Type {
        Null = 0,
        Basic,
        Class,
        Container
    };

    Type type;
    cfuint32 classId;
    CFString ns;
    CFString typeName;
    CFList<SerializeTypeInfo> bases;
    TypeInfoList<SerializeVariableTypeInfo> members;
    TypeInfoList<SerializeFunctionTypeInfo> functions;
    TypeInfoList<SerializeFunctionTypeInfo> cfSignals;

public:
    SerializeTypeInfo() : type(Null), classId(0) {}
    bool operator==(const SerializeTypeInfo & rhs) const { return getName() == rhs.getName(); }
    bool operator<(const SerializeTypeInfo & rhs) const { return getName() < rhs.getName(); }
    CFString toString() const;
    CFString getName() const;
};

class SerializeVariableTypeInfo
{
public:
    CFString name;
    SerializeTypeInfo type;
    bool isRef;

public:
    SerializeVariableTypeInfo() : isRef(false) {}
    SerializeVariableTypeInfo(const CFString & name, const SerializeTypeInfo & type, bool isRef = false) :
        name(name), type(type), isRef(isRef) {}
};

class SerializeFunctionTypeInfo
{
public:
    CFString name;
    SerializeTypeInfo returnType;
    CFList<SerializeVariableTypeInfo> parameters;
    CFList<SerializeVariableTypeInfo> registerParameters;

public:
    CFString toString() const;
    CFString signature(bool withParamNames = false) const;
    bool hasReturnValues() const { return returnValueCount() > 0; }
    uint returnValueCount() const;
};

// Template implementation of TypeInfoList - must be after full type definitions
template<typename T>
struct TypeInfoList<T>::Impl {
    std::vector<T> data;
};

template<typename T> TypeInfoList<T>::TypeInfoList() : impl_(new Impl) {}
template<typename T> TypeInfoList<T>::~TypeInfoList() = default;
template<typename T> TypeInfoList<T>::TypeInfoList(const TypeInfoList & o) : impl_(new Impl(*o.impl_)) {}
template<typename T> TypeInfoList<T>::TypeInfoList(TypeInfoList && o) noexcept = default;
template<typename T> TypeInfoList<T> & TypeInfoList<T>::operator=(const TypeInfoList & o) {
    if (this != &o) impl_ = std::make_unique<Impl>(*o.impl_);
    return *this;
}
template<typename T> TypeInfoList<T> & TypeInfoList<T>::operator=(TypeInfoList && o) noexcept = default;

template<typename T> void TypeInfoList<T>::push_back(const T & val) { impl_->data.push_back(val); }
template<typename T> void TypeInfoList<T>::push_back(T && val) { impl_->data.push_back(std::move(val)); }
template<typename T> void TypeInfoList<T>::clear() { impl_->data.clear(); }
template<typename T> bool TypeInfoList<T>::empty() const { return impl_->data.empty(); }
template<typename T> cfsize_t TypeInfoList<T>::size() const { return impl_->data.size(); }
template<typename T> T & TypeInfoList<T>::operator[](cfsize_t i) { return impl_->data[i]; }
template<typename T> const T & TypeInfoList<T>::operator[](cfsize_t i) const { return impl_->data[i]; }

template<typename T> typename TypeInfoList<T>::iterator TypeInfoList<T>::begin() { return impl_->data.data(); }
template<typename T> typename TypeInfoList<T>::iterator TypeInfoList<T>::end() { return impl_->data.data() + impl_->data.size(); }
template<typename T> typename TypeInfoList<T>::const_iterator TypeInfoList<T>::begin() const { return impl_->data.data(); }
template<typename T> typename TypeInfoList<T>::const_iterator TypeInfoList<T>::end() const { return impl_->data.data() + impl_->data.size(); }

}}    // namespace
