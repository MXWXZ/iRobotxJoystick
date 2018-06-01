/**
 * Copyright (c) 2017-2050
 * All rights reserved.
 *
 * @Author:MXWXZ
 * @Date:2017/12/06
 *
 * @Description:
 */
#pragma once

class MainDlg :public UIWindow {
public:
    MainDlg();
    ~MainDlg();

    void OnInit() override;

private:

protected:
    bool Start(const UIEvent& v_event);
    bool Stop(const UIEvent& v_event);

    static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    MSG_MAP_BEGIN(MainDlg)
        ON_CONTROL_MSG(_T("btn_start"), kWM_LButtonClick, Start)
        ON_CONTROL_MSG(_T("btn_stop"), kWM_LButtonClick, Stop)
        ON_PARENT_MSG(UIWindow)
        MSG_MAP_END
};