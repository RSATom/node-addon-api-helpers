#pragma once

#include <napi.h>

#include "StaticSequence.h"


template<typename T>
inline T FromJsValue(const Napi::Value& value);

template<>
inline Napi::Value FromJsValue<Napi::Value>(const Napi::Value& value)
{
	return value;
}

template<>
inline Napi::Function FromJsValue<Napi::Function>(const Napi::Value& value)
{
	return value.As<Napi::Function>();
}

template<>
inline bool FromJsValue<bool>(const Napi::Value& value)
{
	return value.ToBoolean();
}

template<>
inline unsigned FromJsValue<unsigned>(const Napi::Value& value)
{
	return value.ToNumber();
}

template<>
inline int FromJsValue<int>(const Napi::Value& value)
{
	return value.ToNumber();
}

template <class T>
typename std::enable_if<sizeof(T) < 64, T>::type FromNumber(const Napi::Number& value)
{
	return value.Int32Value();
}

template <class T>
typename std::enable_if<sizeof(T) == 64, T>::type FromNumber(const Napi::Number& value)
{
	return value.Int64Value();
}

template<>
inline long FromJsValue<long>(const Napi::Value& value)
{
	return FromNumber<long>(value.ToNumber());
}

template<>
inline double FromJsValue<double>(const Napi::Value& value)
{
	return value.ToNumber();
}

template<>
inline std::string FromJsValue<std::string>(const Napi::Value& value)
{
	return value.ToString();
}


#if 1

template<typename T>
inline const typename std::enable_if<std::is_enum<T>::value, Napi::Value>::type
ToJsValue(const napi_env& env, const T& value)
{
	return Napi::Value::From(env, static_cast<typename std::underlying_type<T>::type>(value));
}

template<typename T>
const typename std::enable_if<std::is_base_of<Napi::Value, T>::value, Napi::Value>::type
inline ToJsValue(const napi_env& env, const T& value)
{
	return Napi::Value::From(env, value);
}

#else

template<typename T>
inline T ToJsValue(const napi_env& env, const T& value, typename std::enable_if<std::is_enum<T>::value>::type* = 0)
{
	return Napi::Value::From(env, static_cast<typename std::underlying_type<T>::type>(value));
}

template<typename T>
inline T ToJsValue(const napi_env& env, const T& value, typename std::enable_if<std::is_base_of<Napi::Value, T>::value>::type* = 0)
{
	return Napi::Value::From(env, value);
}

#endif

inline Napi::Value ToJsValue(const napi_env& env, bool value)
{
	return Napi::Boolean::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, int value)
{
	return Napi::Number::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, long value)
{
	return Napi::Number::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, unsigned value)
{
	return Napi::Number::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, double value)
{
	return Napi::Number::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, const std::string& value)
{
	return Napi::String::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, const char* value)
{
	return Napi::String::New(env, value);
}

/*
inline Napi::Value ToJsValue(const napi_env& env, const unsigned char* value)
{
	return Napi::String::New(env, reinterpret_cast<const char*>(value));
}
*/


template<typename T1, typename T2>
inline T1 AdjustValue(const T2& x)
{
	return x;
}

template<>
inline const char*
AdjustValue<const char*, std::string>(const std::string& x)
{
	return x.c_str();
}

template<>
inline const unsigned char*
AdjustValue<const unsigned char*, std::string>(const std::string& x)
{
	return reinterpret_cast<const unsigned char*>(x.c_str());
}


template<typename C, typename ... A, size_t ... I>
Napi::Value CallMethod(
	void (C::* method) (A ...),
	const napi_env& env,
	const napi_callback_info& info,
	StaticSequence<I ...>)
{
	Napi::HandleScope scope(env);

	Napi::CallbackInfo callbackInfo(env, info);
	C* instance =  Napi::ObjectWrap<C>::Unwrap(callbackInfo.This().As<Napi::Object>());

	(instance->*method) (
		AdjustValue<A>(
			FromJsValue<
				typename std::conditional<
					std::is_same<A, const char*>::value || std::is_same<A, const unsigned char*>::value,
					std::string,
					typename std::remove_const<
						typename std::remove_reference<A>::type>::type
					>::type
				>(callbackInfo[I])) ...);

	return Napi::Value();
}

template<typename C, typename ... A, size_t ... I>
Napi::Value CallMethod(
	void (C::* method) (const napi_env& env, A ...),
	const napi_env& env,
	const napi_callback_info& info,
	StaticSequence<I ...>)
{
	Napi::HandleScope scope(env);

	Napi::CallbackInfo callbackInfo(env, info);
	C* instance =  Napi::ObjectWrap<C>::Unwrap(callbackInfo.This().As<Napi::Object>());

	(instance->*method) (
		env,
		AdjustValue<A>(
			FromJsValue<
				typename std::conditional<
					std::is_same<A, const char*>::value || std::is_same<A, const unsigned char*>::value,
					std::string,
					typename std::remove_const<
						typename std::remove_reference<A>::type>::type
					>::type
				>(callbackInfo[I])) ...);

	return Napi::Value();
}

template<typename R, typename C, typename ... A, size_t ... I>
Napi::Value CallMethod(
	R (C::* method) (A ...),
	const napi_env& env,
	const napi_callback_info& info,
	StaticSequence<I ...>)
{
	Napi::HandleScope scope(env);

	Napi::CallbackInfo callbackInfo(env, info);
	C* instance =  Napi::ObjectWrap<C>::Unwrap(callbackInfo.This().As<Napi::Object>());

	return
		ToJsValue(env,
			(instance->*method) (
				AdjustValue<A>(
					FromJsValue<
						typename std::conditional<
							std::is_same<A, const char*>::value || std::is_same<A, const unsigned char*>::value,
							std::string,
							typename std::remove_const<
								typename std::remove_reference<A>::type>::type
							>::type
						>(callbackInfo[I])) ...));
}

template<typename R, typename C, typename ... A, size_t ... I>
Napi::Value CallMethod(
	R (C::* method) (const napi_env& env, A ...),
	const napi_env& env,
	const napi_callback_info& info,
	StaticSequence<I ...>)
{
	Napi::HandleScope scope(env);

	Napi::CallbackInfo callbackInfo(env, info);
	C* instance =  Napi::ObjectWrap<C>::Unwrap(callbackInfo.This().As<Napi::Object>());

	return
		ToJsValue(env,
			(instance->*method) (
				env,
				AdjustValue<A>(
					FromJsValue<
						typename std::conditional<
							std::is_same<A, const char*>::value || std::is_same<A, const unsigned char*>::value,
							std::string,
							typename std::remove_const<
								typename std::remove_reference<A>::type>::type
							>::type
						>(callbackInfo[I])) ...));
}

template<typename R, typename C, typename ... A>
napi_value CallMethod(
	R (C::* method) (A ...),
	const napi_env& env,
	const napi_callback_info& info)
{
	typedef typename MakeStaticSequence<sizeof ... ( A )>::SequenceType SequenceType;
	return CallMethod(method, env, info, SequenceType());
}

template<typename R, typename C, typename ... A>
napi_value CallMethod(
	R (C::* method) (const napi_env& env, A ...),
	const napi_env& env,
	const napi_callback_info& info)
{
	typedef typename MakeStaticSequence<sizeof ... ( A )>::SequenceType SequenceType;
	return CallMethod(method, env, info, SequenceType());
}

typedef napi_value (*NapiCallbackType)(napi_env, napi_callback_info);

inline napi_property_descriptor ClassMethod(
	const char* utf8name,
	NapiCallbackType callback,
	napi_property_attributes attributes = napi_default,
	void* data = nullptr)
{
	napi_property_descriptor descriptor {};
	descriptor.utf8name = utf8name;
	descriptor.method = callback;
	descriptor.attributes = attributes;
	descriptor.data = data;
	return descriptor;
}

#define CLASS_METHOD(name, member) \
	ClassMethod(name, \
		[] (napi_env env, napi_callback_info info) -> napi_value { \
			return CallMethod(member, env, info); \
		} \
	)
