#pragma once

#ifdef WQCOLOR_STATIC_DEFINE
#define WQCOLOR_EXPORT
#define WQCOLOR_NO_EXPORT
#else
#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef WQCOLOR_EXPORTS
#define WQCOLOR_EXPORT __declspec(dllexport)
#else
#define WQCOLOR_EXPORT __declspec(dllimport)
#endif
#define WQCOLOR_NO_EXPORT
#elif defined(__GNUC__) || defined(__clang__)
#define WQCOLOR_EXPORT __attribute__((visibility("default")))
#define WQCOLOR_NO_EXPORT __attribute__((visibility("hidden")))
#else
#define WQCOLOR_EXPORT
#define WQCOLOR_NO_EXPORT
#endif
#endif
