#ifndef JOYSTICK_DRIVER
#define JOYSTICK_DRIVER

#include <windows.h>
#include <dinput.h>
#include <dinputd.h>
#include <string>

#include "resource.h"

class Joystick
{
	friend BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
	friend BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);

    public:
		Joystick();
		~Joystick(){FreeDirectInput();}

		HRESULT InitDirectInput( HWND hDlg );
		VOID FreeDirectInput();
		 
		// Stuff to filter out XInput devices
		HRESULT SetupForIsXInputDevice();
		bool IsXInputDevice( const GUID* pGuidProductFromDirectInput );
		void CleanupForIsXInputDevice();

		INT_PTR runJoystick( HWND hDlg);
		HRESULT UpdateInputState(HWND hDlg);

		inline void setAxis(long xA, long yA, long zA, long xR, long yR, long zR, long s0, long s1)
		{ xAxis = xA; yAxis = yA; zAxis = zA; xRot = xR; yRot = yR; zRot = zR; slider0 = s0; slider1 = s1;}
		inline void setPOV(long h0, long h1, long h2, long h3){pov0 = h0; pov1 = h1; pov2 = h2; pov3 = h3;}
		inline void setButton(int num, bool state){button[num] = state;}

		inline UINT getmsg(){return msg;}

		inline int getXAxis(){return xAxis;}
		inline int getYAxis(){return yAxis;}
		inline int getZAxis(){return zAxis;}
		inline int getXRot(){return xRot;}
		inline int getYRot(){return yRot;}
		inline int getZRot(){return zRot;}
		inline int getSlider0(){return slider0;}
		inline int getSlider1(){return slider1;}
		inline int getPOV0(){return pov0;}
		inline int getPOV1(){return pov1;}
		inline int getPOV2(){return pov2;}
		inline int getPOV3(){return pov3;}

		inline bool getButton(int num){return button[num];}

		inline std::string getErrorMSG(){return errorMSG;}

	private:
		struct XINPUT_DEVICE_NODE
		{
		DWORD dwVidPid;
		XINPUT_DEVICE_NODE* pNext;
		};

		struct DI_ENUM_CONTEXT
		{
		DIJOYCONFIG* pPreferredJoyCfg;
		bool bPreferredJoyCfgValid;
		};

		UINT msg;
		int timeOut;

		bool g_bFilterOutXinputDevices;

		XINPUT_DEVICE_NODE*     g_pXInputDeviceList;
		LPDIRECTINPUT8          g_pDI;
		LPDIRECTINPUTDEVICE8    g_pJoystick;

		long xAxis;
		long yAxis;
		long zAxis;

		long xRot;
		long yRot;
		long zRot;

		long slider0;
		long slider1;

		long pov0;
		long pov1;
		long pov2;
		long pov3;

		bool button[128];
		
		std::string errorMSG;
};

#endif