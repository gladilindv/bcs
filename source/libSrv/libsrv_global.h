#pragma once
#ifdef LIBSRV_EXPORTS
# define LIBSRV_EXPORT __declspec(dllexport)
#else
# define LIBSRV_EXPORT __declspec(dllimport)
#endif