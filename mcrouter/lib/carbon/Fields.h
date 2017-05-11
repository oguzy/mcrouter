/*
 *  Copyright (c) 2017, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <string>
#include <vector>

#include <folly/Traits.h>

#include "mcrouter/lib/carbon/SerializationTraits.h"

namespace carbon {

enum class FieldType : uint8_t {
  Stop = 0x0,
  True = 0x1,
  False = 0x2,
  Int8 = 0x3,
  Int16 = 0x4,
  Int32 = 0x5,
  Int64 = 0x6,
  Double = 0x7,
  Binary = 0x8,
  List = 0x9,
  Set = 0xa,
  Map = 0xb,
  Struct = 0xc,
  Float = 0xd
};

template <class T>
class IsCarbonStruct {
  template <class C>
  static constexpr decltype(&C::serialize, std::true_type()) check(int);

  template <class C>
  static constexpr std::false_type check(...);

 public:
  static constexpr bool value{decltype(check<T>(0))::value};
};

template <class T>
class IsCarbonUnion {
  template <class C>
  static constexpr decltype(&C::which, std::true_type()) check(int);

  template <class C>
  static constexpr std::false_type check(...);

 public:
  static constexpr bool value{decltype(check<T>(0))::value};
};

namespace detail {

template <class T>
class IsUserReadWriteDefined {
  template <class C>
  static constexpr decltype(
      SerializationTraits<C>::read,
      SerializationTraits<C>::write,
      std::true_type())
  check(int);

  template <class C>
  static constexpr std::false_type check(...);

 public:
  static constexpr bool value{decltype(check<T>(0))::value};
};

template <class T>
class IsSerializableViaTraits {
  template <class C>
  static constexpr decltype(SerializationTraits<C>::kWireType, std::true_type())
  check(int);

  template <class C>
  static constexpr std::false_type check(...);

 public:
  static constexpr bool value{decltype(check<T>(0))::value};
};

template <class T, FieldType Type, typename Enable = void>
struct IsOfTraitFieldType;

template <class T, FieldType Type>
struct IsOfTraitFieldType<
    T,
    Type,
    typename std::enable_if<IsSerializableViaTraits<T>::value>::type> {
  static constexpr bool value = SerializationTraits<T>::kWireType == Type;
};

template <class T, FieldType Type>
struct IsOfTraitFieldType<
    T,
    Type,
    typename std::enable_if<!IsSerializableViaTraits<T>::value>::type> {
  static constexpr bool value = false;
};

template <class T>
struct IsLinearContainer {
  static constexpr bool value = IsOfTraitFieldType<T, FieldType::List>::value ||
      IsOfTraitFieldType<T, FieldType::Set>::value;
};

template <class T>
struct IsKVContainer {
  static constexpr bool value = IsOfTraitFieldType<T, FieldType::Map>::value;
};

template <class T, class Enable = void>
struct TypeToField {};

template <>
struct TypeToField<bool> {
  static constexpr FieldType fieldType{FieldType::True};
};

template <>
struct TypeToField<float> {
  static constexpr FieldType fieldType{FieldType::Float};
};

template <>
struct TypeToField<double> {
  static constexpr FieldType fieldType{FieldType::Double};
};

template <class T>
struct TypeToField<
    T,
    typename std::enable_if<
        folly::IsOneOf<T, char, int8_t, uint8_t>::value>::type> {
  static constexpr FieldType fieldType{FieldType::Int8};
};

template <class T>
struct TypeToField<
    T,
    typename std::enable_if<
        folly::IsOneOf<T, int16_t, uint16_t>::value>::type> {
  static constexpr FieldType fieldType{FieldType::Int16};
};

template <class T>
struct TypeToField<
    T,
    typename std::enable_if<
        folly::IsOneOf<T, int32_t, uint32_t>::value>::type> {
  static constexpr FieldType fieldType{FieldType::Int32};
};

template <class T>
struct TypeToField<
    T,
    typename std::enable_if<
        folly::IsOneOf<T, int64_t, uint64_t>::value>::type> {
  static constexpr FieldType fieldType{FieldType::Int64};
};

template <class T>
struct TypeToField<T, typename std::enable_if<std::is_enum<T>::value>::type> {
  static constexpr FieldType fieldType{
      TypeToField<typename std::underlying_type<T>::type>::fieldType};
};

template <class T>
struct TypeToField<T, typename std::enable_if<IsCarbonStruct<T>::value>::type> {
  static constexpr FieldType fieldType{FieldType::Struct};
};

template <class T>
struct TypeToField<
    T,
    typename std::enable_if<IsSerializableViaTraits<T>::value>::type> {
  static constexpr FieldType fieldType{SerializationTraits<T>::kWireType};
};

} // detail
} // carbon
