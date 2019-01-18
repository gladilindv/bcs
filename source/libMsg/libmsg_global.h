#pragma once
#ifdef LIBMSG_EXPORTS
# define LIBMSG_EXPORT __declspec(dllexport)
#else
# define LIBMSG_EXPORT __declspec(dllimport)
#endif