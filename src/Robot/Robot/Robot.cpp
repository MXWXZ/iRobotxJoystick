#include "stdafx.h"
#include "Robot.h"
#include "MainDlg.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPTSTR    lpCmdLine,
                      _In_ int       nCmdShow) {
    UISystem::Init(hInstance);      // Initialize UI library
    UIRender::GlobalInit();         // Init render
    UIConfig::LoadConfig();         // Load main config

    {                       // Please write code between the brackets
        MainDlg dlg;        // Show main dlg
        dlg.Run();
    }

    UISystem::Cleanup();            // Clean up UI library
    return 0;
}
