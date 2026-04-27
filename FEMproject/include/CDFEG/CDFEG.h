#ifdef _WIN32
#ifdef CDFEG_EXPORTS
#define CDFEG_API __declspec(dllexport)
#else
#define CDFEG_API __declspec(dllimport)
#endif
#else
#define CDFEG_API
//#define CDFEG_API __attribute__((visibility("default")))
#endif
