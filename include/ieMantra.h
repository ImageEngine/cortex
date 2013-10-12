#ifndef ieMantra_h
#define ieMantra_h
namespace IECoreMantra
{
	// Begin Microsoft Windows DLL support.
#if defined(IECoreMantra_EXPORTS)
	// For the DLL library.
#define CortexManAPI __declspec(dllexport)
#elif defined(IECoreMantra_IMPORTS)
	// For a client of the DLL library.
#define CortexManAPI __declspec(dllimport)
#else
	// For the static library and for Apple/Linux.
#define CortexManAPI
#endif
	// End Microsoft Windows DLL support.
}
#endif