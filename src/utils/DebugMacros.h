#ifndef DEBUG_MACROS_H
#define DEBUG_MACROS_H

#include <iostream>

#ifdef _DEBUG
#define DEBUG_MODE
#endif

#ifdef DEBUG_MODE
#define DEBUG_LOG(x) std::wcerr << L"DEBUG: " << x << std::endl
#else
#define DEBUG_LOG(x)
#endif

#endif // DEBUG_MACROS_H