#include "Joystick_Driver.h"

#include <windows.h>
#include <commctrl.h>
#include <dinput.h>
#include <dinputd.h>
#include <assert.h>

#pragma warning( disable : 4996 ) // disable deprecated warning 
#pragma warning( default : 4996 )
#include "resource.h"

#include <wbemidl.h>

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

//the friend functions need the equivalent of a "this" pointer
namespace External {
	 Joystick* exJS = new Joystick(); //points to dummy until External::exJS = this; is defined
}

Joystick::Joystick()
{
	g_bFilterOutXinputDevices = false;
	g_pXInputDeviceList = NULL;
	g_pDI = NULL;
	g_pJoystick = NULL;

	msg = WM_INITDIALOG;
	
	xAxis = 0;
	yAxis = 0;
	zAxis = 0;
	xRot = 0;
	yRot = 0;
	zRot = 0;
	slider0 = 0;
	slider1 = 0;
	pov0 = 0;
	pov1 = 0;
	pov2 = 0;
	pov3 = 0;

	for(int i=0; i < 128; i++)
	{
		button[i] = false;
	}
	InitCommonControls();
	
	WCHAR* strCmdLine;
    int nNumArgs;
    LPWSTR* pstrArgList = CommandLineToArgvW( GetCommandLineW(), &nNumArgs );
    for( int iArg = 1; iArg < nNumArgs; iArg++ )
    {
        strCmdLine = pstrArgList[iArg];

        // Handle flag args
        if( *strCmdLine == L'/' || *strCmdLine == L'-' )
        {
            strCmdLine++;

            int nArgLen = ( int )wcslen( L"noxinput" );
            if( _wcsnicmp( strCmdLine, L"noxinput", nArgLen ) == 0 && strCmdLine[nArgLen] == 0 )
            {
                g_bFilterOutXinputDevices = true;
                continue;
            }
        }
    }
    LocalFree( pstrArgList );

	External::exJS = this; //copy a pointer of the newly constructed object into the 
}


//-----------------------------------------------------------------------------
// Name: MainDialogProc
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR Joystick::runJoystick( HWND hDlg)
{

    switch( this->msg )
    {
        case WM_INITDIALOG:
            InitDirectInput( hDlg );
			return TRUE;

        case WM_TIMER:
            // Update the input device
            UpdateInputState( hDlg);
			return TRUE;

        case WM_DESTROY:
            // Cleanup everything
            FreeDirectInput();
			return TRUE;
    }

    return FALSE; // Message not handled 
}



//-----------------------------------------------------------------------------
// Name: InitDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT Joystick::InitDirectInput( HWND hDlg)
{
    HRESULT hr;

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&this->g_pDI, NULL ) ) )
        return hr;

    if( this->g_bFilterOutXinputDevices )
        SetupForIsXInputDevice();

    DIJOYCONFIG PreferredJoyCfg = {0};
    DI_ENUM_CONTEXT enumContext;
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
    enumContext.bPreferredJoyCfgValid = false;

    IDirectInputJoyConfig8* pJoyConfig = NULL;
    if( FAILED( hr = this->g_pDI->QueryInterface( IID_IDirectInputJoyConfig8, ( void** )&pJoyConfig ) ) )
        return hr;

    PreferredJoyCfg.dwSize = sizeof( PreferredJoyCfg );
    if( SUCCEEDED( pJoyConfig->GetConfig( 0, &PreferredJoyCfg, DIJC_GUIDINSTANCE ) ) ) // This function is expected to fail if no Joystick is attached
        enumContext.bPreferredJoyCfgValid = true;
    SAFE_RELEASE( pJoyConfig );
 
    // Look for a simple Joystick we can use for this sample program.
    if( FAILED( hr = this->g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &enumContext, DIEDFL_ATTACHEDONLY ) ) )
        return hr;

    if( this->g_bFilterOutXinputDevices )
        CleanupForIsXInputDevice();

    // Make sure we got a Joystick
    if( NULL == this->g_pJoystick )
    {
		this->errorMSG = "Joystick Not Found";
        return S_OK;
    }

    // Set the data format to "simple Joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if( FAILED( hr = this->g_pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
        return hr;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    if( FAILED( hr = this->g_pJoystick->SetCooperativeLevel( hDlg, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
        return hr;

    // Enumerate the Joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    if( FAILED( hr = this->g_pJoystick->EnumObjects( EnumObjectsCallback, ( VOID* )hDlg, DIDFT_ALL ) ) )
        return hr;

	this->errorMSG = "Joystick Connected Ok";
	this->msg = WM_TIMER;
    return S_OK;
}


//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains 
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it�s an XInput device
// Unfortunately this information can not be found by just using DirectInput.
// Checking against a VID/PID of 0x028E/0x045E won't find 3rd party or future 
// XInput devices.
//
// This function stores the list of xinput devices in a linked list 
// at g_pXInputDeviceList, and IsXInputDevice() searchs that linked list
//-----------------------------------------------------------------------------
HRESULT Joystick::SetupForIsXInputDevice()
{
    IWbemServices* pIWbemServices = NULL;
    IEnumWbemClassObject* pEnumDevices = NULL;
    IWbemLocator* pIWbemLocator = NULL;
    IWbemClassObject* pDevices[20] = {0};
    BSTR bstrDeviceID = NULL;
    BSTR bstrClassName = NULL;
    BSTR bstrNamespace = NULL;
    DWORD uReturned = 0;
    bool bCleanupCOM = false;
    UINT iDevice = 0;
    VARIANT var;
    HRESULT hr;

    // CoInit if needed
    hr = CoInitialize( NULL );
    bCleanupCOM = SUCCEEDED( hr );

    // Create WMI
    hr = CoCreateInstance( __uuidof( WbemLocator ), NULL, CLSCTX_INPROC_SERVER, __uuidof( IWbemLocator ), ( LPVOID* )&pIWbemLocator );
    if( FAILED( hr ) || pIWbemLocator == NULL )
        goto LCleanup;

    // Create BSTRs for WMI
    bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" ); if( bstrNamespace == NULL ) goto LCleanup;
    bstrDeviceID = SysAllocString( L"DeviceID" );           if( bstrDeviceID == NULL )  goto LCleanup;
    bstrClassName = SysAllocString( L"Win32_PNPEntity" );    if( bstrClassName == NULL ) goto LCleanup;

    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices );
    if( FAILED( hr ) || pIWbemServices == NULL )
        goto LCleanup;

    // Switch security level to IMPERSONATE
    CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0 );

    // Get list of Win32_PNPEntity devices
    hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices );
    if( FAILED( hr ) || pEnumDevices == NULL )
        goto LCleanup;

    // Loop over all devices
    for(; ; )
    {
        // Get 20 at a time
        hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
        if( FAILED( hr ) )
            goto LCleanup;
        if( uReturned == 0 )
            break;

        for( iDevice = 0; iDevice < uReturned; iDevice++ )
        {
            // For each device, get its device ID
            hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
            if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
            {
                // Check if the device ID contains "IG_".  If it does, then it�s an XInput device
                // Unfortunately this information can not be found by just using DirectInput 
                if( wcsstr( var.bstrVal, L"IG_" ) )
                {
                    // If it does, then get the VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                    if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                        dwVid = 0;
                    WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                    if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
                        dwPid = 0;

                    DWORD dwVidPid = MAKELONG( dwVid, dwPid );

                    // Add the VID/PID to a linked list
                    XINPUT_DEVICE_NODE* pNewNode = new XINPUT_DEVICE_NODE;
                    if( pNewNode )
                    {
                        pNewNode->dwVidPid = dwVidPid;
                        pNewNode->pNext = this->g_pXInputDeviceList;
                        this->g_pXInputDeviceList = pNewNode;
                    }
                }
            }
            SAFE_RELEASE( pDevices[iDevice] );
        }
    }

LCleanup:
    if( bstrNamespace )
        SysFreeString( bstrNamespace );
    if( bstrDeviceID )
        SysFreeString( bstrDeviceID );
    if( bstrClassName )
        SysFreeString( bstrClassName );
    for( iDevice = 0; iDevice < 20; iDevice++ )
    SAFE_RELEASE( pDevices[iDevice] );
    SAFE_RELEASE( pEnumDevices );
    SAFE_RELEASE( pIWbemLocator );
    SAFE_RELEASE( pIWbemServices );

    return hr;
}

//-----------------------------------------------------------------------------
// Name: EnumJoysticksCallback()
// Desc: Called once for each enumerated Joystick. If we find one, create a
//       device interface on it so we can play with it.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    Joystick::DI_ENUM_CONTEXT* pEnumContext = (Joystick::DI_ENUM_CONTEXT* )pContext;
    HRESULT hr;

    if( External::exJS->g_bFilterOutXinputDevices && External::exJS->IsXInputDevice( &pdidInstance->guidProduct ) )
        return DIENUM_CONTINUE;

    // Skip anything other than the perferred Joystick device as defined by the control panel.  
    // Instead you could store all the enumerated Joysticks and let the user pick.
    if( pEnumContext->bPreferredJoyCfgValid &&
        !IsEqualGUID( pdidInstance->guidInstance, pEnumContext->pPreferredJoyCfg->guidInstance ) )
        return DIENUM_CONTINUE;

    // Obtain an interface to the enumerated Joystick.
    hr = External::exJS->g_pDI->CreateDevice( pdidInstance->guidInstance, &External::exJS->g_pJoystick, NULL );

    // If it failed, then we can't use this Joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( FAILED( hr ) )
        return DIENUM_CONTINUE;

    // Stop enumeration. Note: we're just taking the first Joystick we get. You
    // could store all the enumerated Joysticks and let the user pick.
    return DIENUM_STOP;
}


//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       Joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
    HWND hDlg = ( HWND )pContext;

    static int nSliderCount = 0;  // Number of returned slider controls
    static int nPOVCount = 0;     // Number of returned POV controls

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg;
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin = -1000;
        diprg.lMax = +1000;

        // Set the range for the axis
        if( FAILED( External::exJS->g_pJoystick->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
            return DIENUM_STOP;

    }
	
    return DIENUM_CONTINUE;
}


//-----------------------------------------------------------------------------
// Returns true if the DirectInput device is also an XInput device.
// Call SetupForIsXInputDevice() before, and CleanupForIsXInputDevice() after
//-----------------------------------------------------------------------------
bool Joystick::IsXInputDevice( const GUID* pGuidProductFromDirectInput )
{
    // Check each xinput device to see if this device's vid/pid matches
    XINPUT_DEVICE_NODE* pNode = this->g_pXInputDeviceList;
    while( pNode )
    {
        if( pNode->dwVidPid == pGuidProductFromDirectInput->Data1 )
            return true;
        pNode = pNode->pNext;
    }

    return false;
}


//-----------------------------------------------------------------------------
// Cleanup needed for IsXInputDevice()
//-----------------------------------------------------------------------------
void Joystick::CleanupForIsXInputDevice()
{
    // Cleanup linked list
    XINPUT_DEVICE_NODE* pNode = this->g_pXInputDeviceList;
    while( pNode )
    {
        XINPUT_DEVICE_NODE* pDelete = pNode;
        pNode = pNode->pNext;
        SAFE_DELETE( pDelete );
    }
}



//-----------------------------------------------------------------------------
// Name: UpdateInputState()
// Desc: Get the input device's state and display it.
//-----------------------------------------------------------------------------
HRESULT Joystick::UpdateInputState( HWND hDlg)
{
    HRESULT hr;
    TCHAR strText[512] = {0}; // Device state text
    DIJOYSTATE2 js;           // DInput Joystick state 

   
	if( NULL == this->g_pJoystick )
        return S_OK;

    // Poll the device to read the current state
    hr = this->g_pJoystick->Poll();
    if( FAILED( hr ) )
    {
        // DInput is telling us that the input stream has been
        // interrupted. We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done. We
        // just re-acquire and try again.
        hr = this->g_pJoystick->Acquire();
        if( hr == DIERR_INPUTLOST )
		{
			this->g_pJoystick->Acquire();
			return hr;
		}
        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return S_OK;
    }

    // Get the input's device state
    if( FAILED( hr = this->g_pJoystick->GetDeviceState( sizeof( DIJOYSTATE2 ), &js ) ) )
	{
		this->errorMSG = "Joystick connection lost";
		this->msg = WM_DESTROY; //find a new joystick
        return hr;
	}
    //Read values into Joystick object
	this->setAxis(js.lX, js.lY, js.lZ, js.lRx, js.lRy, js.lRz, js.rglSlider[0], js.rglSlider[1]);
	
	this->setPOV(js.rgdwPOV[0], js.rgdwPOV[1], js.rgdwPOV[2], js.rgdwPOV[3]);
	
    for( int i = 0; i < 128; i++ )
    {
        if( js.rgbButtons[i] & 0x80 )
		this->setButton(i,true);
        else
		this->setButton(i,false);
    }

	this->errorMSG = "Reading Joystick OK";
    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
VOID Joystick::FreeDirectInput()
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if(this->g_pJoystick )
        this->g_pJoystick->Unacquire();

    // Release any DirectInput objects.
    SAFE_RELEASE( this->g_pJoystick );
    SAFE_RELEASE( this->g_pDI );
	this->msg = WM_INITDIALOG;
}

