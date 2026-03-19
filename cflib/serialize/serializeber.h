/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/impl/serializeberimpl.h>

namespace cflib::serialize {

class BERSerializer
{
public:
    BERSerializer(uint64 tagNo = 0) : lenPos_(0), data_(), base_(data_) {
        if (tagNo > 0) {
            const uint8 tagLen = impl::calcTagLen(tagNo);
            data_.resize(tagLen + 1);
            uint8 * pos = (uint8 *)data_.data();
            impl::writeTagBytes(pos, tagNo, true, tagLen);
            pos[tagLen] = '\0';
            lenPos_ = tagLen + 1;
        }
    }

    ByteArray data() {
        if (lenPos_) {
            impl::insertBERLength(data_, lenPos_);
            lenPos_ = 0;
        }
        return data_;
    }

    template<typename T>
    inline BERSerializer & operator<<(const T & cl) { base_ << cl; return *this; }

    inline BERSerializer & operator<<(Placeholder ph) { base_ << ph; return *this; }

private:
    int lenPos_;
    ByteArray data_;
    impl::BERSerializerBase base_;
};

class BERDeserializer
{
public:
    BERDeserializer(const ByteArray & data) :
        data_(data), base_((const uint8 *)data_.constData(), data_.size()) {}
    BERDeserializer(const ByteArray & ba, const uint8 * data, int len) :
        data_(ba), base_(data, len) {}

    template<typename T>
    inline BERDeserializer & operator>>(T & cl) { base_ >> cl; return *this; }

    inline BERDeserializer & operator>>(Placeholder ph) { base_ >> ph; return *this; }

    template<typename T>
    inline T get() { T retval; base_ >> retval; return std::move(retval); }

    inline bool isAnyAvailable() const { return base_.isAnyAvailable(); }

private:
    const ByteArray data_;
    impl::BERDeserializerBase base_;
};

} // namespace
