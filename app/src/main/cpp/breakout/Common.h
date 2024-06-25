

#ifndef TESTBREAKOUT_COMMON_H
#define TESTBREAKOUT_COMMON_H

#include "Core/BasicTypes.h"

#define NODISCARD [[nodiscard]]

#ifndef DEFAULT_COPY
#define DEFAULT_COPY(T) \
	T(const T&) = default; \
	T& operator=(const T&) = default;
#endif

#ifndef DEFAULT_MOVE
#define DEFAULT_MOVE(T) \
	T(T&&) noexcept = default; \
	T& operator=(T&&) = default;
#endif


#ifndef DEFAULT_MOVE_AND_COPY
#define DEFAULT_MOVE_AND_COPY(T) \
		DEFAULT_MOVE(T) \
		DEFAULT_COPY(T)
#endif


#ifndef DISABLE_COPY
#define DISABLE_COPY(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete;
#endif

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T) \
	T(T&&) = delete; \
	T& operator=(T&&) = delete;
#endif

#ifndef DISABLE_MOVE_AND_COPY
#define DISABLE_MOVE_AND_COPY(T) \
	DISABLE_MOVE(T) \
	DISABLE_COPY(T)
#endif
#endif //TESTBREAKOUT_COMMON_H
