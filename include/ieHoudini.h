#ifndef ieHoudini_h
#define ieHoudini_h

namespace IECoreHoudini
{
	// Begin Microsoft Windows DLL support.
#if defined(IECoreHoudini_EXPORTS)
	// For the DLL library.
#define CortexHOUAPI __declspec(dllexport)
#elif defined(IECoreHoudini_IMPORTS)
	// For a client of the DLL library.
#define CortexHOUAPI __declspec(dllimport)
#else
	// For the static library and for Apple/Linux.
#define CortexHOUAPI
#endif
	// End Microsoft Windows DLL support.
}
#endif