#ifdef _WIN32
#ifdef SIFEG_EXPORTS
#define SIFEG_API __declspec(dllexport)
#else
#define SIFEG_API __declspec(dllimport)
#endif
#else
#define SIFEG_API
//#define SIFEG_API __attribute__((visibility("default")))
#endif
