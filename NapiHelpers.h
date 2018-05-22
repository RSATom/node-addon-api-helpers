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


inline Napi::Value ToJsValue(const Napi::Value& value)
{
	return value;
}

inline Napi::Value ToJsValue(const napi_env& env, bool value)
{
	return Napi::Boolean::New(env, value);
}

inline Napi::Value ToJsValue(const napi_env& env, int value)
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


template<typename T>
inline T AdjustValue(T x)
{
	return x;
}

inline const char* AdjustValue(const std::string& x)
{
	return x.c_str();
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
		AdjustValue(
			FromJsValue<
				std::conditional<
					std::is_same<A, const char*>::value,
					std::string,
					typename std::remove_const<
						typename std::remove_reference<A>::type>::type
					>::type
				>(callbackInfo[I])) ...);

	return Napi::Value();
};

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
				AdjustValue(
					FromJsValue<
						std::conditional<
							std::is_same<A, const char*>::value,
							std::string,
							typename std::remove_const<
								typename std::remove_reference<A>::type>::type
							>::type
						>(callbackInfo[I])) ...));
};

template<typename R, typename C, typename ... A>
napi_value CallMethod(
	R (C::* method) (A ...),
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
};

#define CLASS_METHOD(name, member) \
	ClassMethod(name, \
		[] (napi_env env, napi_callback_info info) -> napi_value { \
			return CallMethod(member, env, info); \
		} \
	)
