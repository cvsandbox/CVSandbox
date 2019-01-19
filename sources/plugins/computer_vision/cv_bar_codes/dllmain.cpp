#include <windows.h>
#include <xtypes.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    XUNREFERENCED_PARAMETER( hModule )
    XUNREFERENCED_PARAMETER( lpReserved )

    switch ( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // http://connect.microsoft.com/VisualStudio/feedback/details/574669/main-program-crashes-after-unloading-a-dll-library-that-uses-openmp
#ifdef _MSC_VER
        Sleep( 1000 );
#endif
        break;
    }
	return TRUE;
}
