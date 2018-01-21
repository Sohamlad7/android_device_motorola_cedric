/*  -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
    vi:ai:tabstop=8:shiftwidth=4:softtabstop=4:expandtab

    The two lines above are (X)Emacs and Vi(m) modelines to configure consistent indentation.
    This file commented for Doxygen. For details about Doxygen, go to http://www.doxygen.org */

/** \file
 *  \brief Defines a class to handle type-safe endian conversion methods.
 *
 *                           Motorola Confidential Restricted
 *                    (c) Copyright Motorola 2015, All Rights Reserved
 *
 */

#ifndef _ENDIAN_HPP
#define _ENDIAN_HPP

#include <cstdint>
#include <type_traits>
#include <assert.h>

#undef BYTE_SWAP_16
#undef BYTE_SWAP_32
#undef BYTE_SWAP_64

/** Non-optimized way to swap two bytes. */
#define BYTE_SWAP_16(x) \
    ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

/** Non-optimized way to swap four bytes. */
#define BYTE_SWAP_32(x) \
    ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

/** Non-optimized way to swap eight bytes. */
#define BYTE_SWAP_64(x) \
    ((((x) & 0xff00000000000000ull) >> 56) | (((x) & 0x00ff000000000000ull) >> 40) | \
     (((x) & 0x0000ff0000000000ull) >> 24) | (((x) & 0x000000ff00000000ull) >>  8) | \
     (((x) & 0x00000000ff000000ull) <<  8) | (((x) & 0x0000000000ff0000ull) << 24) | \
     (((x) & 0x000000000000ff00ull) << 40) | (((x) & 0x00000000000000ffull) << 56))


class Endian {
public:
    /** Type-safe way to swap the byte order for standard int types. */
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type >
    T static inline swap(T val) {
        switch (sizeof(T)) {
            case 2: return BYTE_SWAP_16(val);
            case 4: return BYTE_SWAP_32(val);
            case 8: return BYTE_SWAP_64(val);
            default: return val;
        }
    }

    /** Converts from network to host byte order (or vice versa). */
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type >
    T static inline ntoh(T val) {
        return bigEndian ? val : swap(val);
    }

    /** Checks the platform byte-order at run-time.
     *
     * @return True if the platform is big-endian (network byte order).
     * */
    static bool isBigEndian() { return bigEndian; }

    /** Extracts a standard integer type out of a byte array in a type-safe
     * manner. The byte array must store the value in network byte order. The
     * value is converted to the host byte order upon extraction.
     *
     * This base function should only be called if a better member function
     * template specialization doesn't exist (specializations are below).
     * Notice this function doesn't return anything even though it declares a
     * return type, so that a compiler warning is generated in case it is used.
     */
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type >
    static inline T extract(uint8_t* a) {
        assert(false);
    }

    /** Extracts a standard integer type out of a byte array in a type-safe
     * manner. The byte array must store the value in little endian byte order.
     * The value is converted to the host byte order upon extraction.
     */
    template<typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type >
    static inline T extractLittleEndian(uint8_t* a) {
        return swap(extract<T>(a));
    }

private:
    typedef union {
        uint32_t val;
        uint8_t v[4];
    } TestUnion;
    static const TestUnion test;
    static const bool bigEndian;
};

//@{
/** Member function template specializations for Endian::extract().
 */
template<> inline int8_t Endian::extract<int8_t>(uint8_t* a) {
    return static_cast<int8_t>(a[0]);
}

template<> inline uint8_t Endian::extract<uint8_t>(uint8_t* a) {
    return a[0];
}

template<> inline int16_t Endian::extract<int16_t>(uint8_t* a) {
    return  static_cast<int16_t>(a[0]) << 8 |
            static_cast<int16_t>(a[1]);
}

template<> inline uint16_t Endian::extract<uint16_t>(uint8_t* a) {
    return  static_cast<uint16_t>(a[0]) << 8 |
            static_cast<uint16_t>(a[1]);
}

template<> inline int32_t Endian::extract<int32_t>(uint8_t* a) {
    return  static_cast<int32_t>(a[0]) << 24 |
            static_cast<int32_t>(a[1]) << 16 |
            static_cast<int32_t>(a[2]) << 8 |
            static_cast<int32_t>(a[3]);
}

template<> inline uint32_t Endian::extract<uint32_t>(uint8_t* a) {
    return  static_cast<uint32_t>(a[0]) << 24 |
            static_cast<uint32_t>(a[1]) << 16 |
            static_cast<uint32_t>(a[2]) << 8 |
            static_cast<uint32_t>(a[3]);
}

template<> inline int64_t Endian::extract<int64_t>(uint8_t* a) {
    return  static_cast<int64_t>(a[0]) << 56 |
            static_cast<int64_t>(a[1]) << 48 |
            static_cast<int64_t>(a[2]) << 40 |
            static_cast<int64_t>(a[3]) << 32 |
            static_cast<int64_t>(a[4]) << 24 |
            static_cast<int64_t>(a[5]) << 16 |
            static_cast<int64_t>(a[6]) << 8 |
            static_cast<int64_t>(a[7]);
}

template<> inline uint64_t Endian::extract<uint64_t>(uint8_t* a) {
    return  static_cast<uint64_t>(a[0]) << 56 |
            static_cast<uint64_t>(a[1]) << 48 |
            static_cast<uint64_t>(a[2]) << 40 |
            static_cast<uint64_t>(a[3]) << 32 |
            static_cast<uint64_t>(a[4]) << 24 |
            static_cast<uint64_t>(a[5]) << 16 |
            static_cast<uint64_t>(a[6]) << 8 |
            static_cast<uint64_t>(a[7]);
}
//@}

#endif // _ENDIAN_HPP
