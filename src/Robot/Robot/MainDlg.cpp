#include "stdafx.h"
#include "Robot.h"
#include "MainDlg.h"
#include "Joystick_Driver.h"
#include "winsock2.h"
#include <cmath>
#pragma comment(lib,"ws2_32.lib")

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"comctl32.lib")

MainDlg::MainDlg()
    :UIWindow(_T("main")) {
}

MainDlg::~MainDlg() {}

void MainDlg::OnInit() {
    SetIcon(IDI_ROBOT);
    UIWindow::OnInit();
}

WSADATA wsaData;
SOCKADDR_IN addrSrv;
SOCKET sockClient;
UIWindow* wnd = nullptr;
Joystick* JS = nullptr;
#define TIMER_ID 1
//#define DEBUG_MODE

bool MainDlg::Start(const UIEvent & v_event) {
    UIButton* btn_start = (UIButton*)FindCtrlFromName(_T("btn_start"));
    UIButton* btn_stop = (UIButton*)FindCtrlFromName(_T("btn_stop"));
    btn_start->DisableCtrl();
    btn_stop->DisableCtrl(FALSE);
    JS = new Joystick();
    wnd = this;
    SetTimer(GetHWND(), TIMER_ID, 35, TimerProc);
    UIText* txt_state = (UIText*)wnd->FindCtrlFromName(_T("txt_state"));
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        txt_state->SetText(_T("初始化Winsock失败"));
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(8266);
    addrSrv.sin_addr.S_un.S_addr = inet_addr("192.168.4.1");
    sockClient = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == sockClient)
        txt_state->SetText(_T("Socket() error"));
#ifndef DEBUG_MODE
    if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET)
        txt_state->SetText(_T("连接失败:%d"));
#endif
    UpdateWindow();
    return true;
}

bool MainDlg::Stop(const UIEvent & v_event) {
    UIButton* btn_start = (UIButton*)FindCtrlFromName(_T("btn_start"));
    UIButton* btn_stop = (UIButton*)FindCtrlFromName(_T("btn_stop"));
    btn_start->DisableCtrl(FALSE);
    btn_stop->DisableCtrl();
    JS->FreeDirectInput();
    KillTimer(GetHWND(), TIMER_ID);
    closesocket(sockClient);
    WSACleanup();
    UpdateWindow();
    return true;
}

int rec_xaxis = 0, rec_yaxis = 0, rec_zaxis=0 ,rec_pov1 = -1, rec_lbtn = 0;
int rec_abutton = 0, rec_xbutton = 0, rec_bbutton = 0, rec_ybutton = 0;
bool flg_lbtn = false, flg_rbtn = false, flg_abtn = false;
bool isstart = false;
int step = 3;
bool issing = false;
int motor = 0, sidemotor = 0, vac = 0;
unsigned char buffer[128];
#define PI 3.1415926
#define SRIGHT(x) buffer[1]=x>=0?0:255;buffer[2]=x>=0?x:256+x;
#define SLEFT(x) buffer[3]=x>=0?0:255;buffer[4]=x>=0?x:256+x;
#define OFFSET ((double)step / 3)
#define SEND(x) send(sockClient, (char*)buffer, x, 0); 

DWORD WINAPI Sing(LPVOID lpParamter) {
    char buffs[][50] = { { 140,0,16, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5 },
    { 140,1,16, 62, 5, 60, 5, 65, 10, 64, 5, 65, 5, 62, 10, 60, 10, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5 },
    { 140,2,16, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 72, 10, 77, 10, 76, 5, 77, 5, 76, 5, 74, 5, 72, 10, 69, 10, 67, 10, 69, 10, 62, 5 },
    { 140,3,16, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 65, 10 },
    { 140,0,16, 64, 5, 65, 5, 62, 10, 60, 10, 62, 10, 60, 5, 62, 5, 65, 10, 62, 5, 65, 5, 69, 10, 71, 5, 74, 5, 76, 5, 77, 5, 72, 5 },
    { 140,1,16,77, 10, 76, 5, 77, 5, 74, 10, 72, 10, 74, 20, 62, 10, 64, 10, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10 },
    { 140,2,16,62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 65, 10, 64, 5, 65, 5, 62, 10, 60, 10, 67, 10 },
    { 140,3,16,69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 72, 10, 77, 10, 76, 5 },
    { 140,0,16,77, 5, 76, 5, 74, 5, 72, 10, 69, 10, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5 },
    { 140,1,16,60, 5, 67, 10, 69, 10, 62, 5, 60, 5, 62, 5, 60, 5, 65, 10, 64, 5, 65, 5, 62, 10, 60, 10, 62, 5, 65, 5, 69, 5, 72, 5 },
    { 140,2,12,77, 5, 76, 5, 72, 5, 69, 5, 67, 10, 65, 10, 67, 10, 69, 10, 62, 15, 62, 15, 60, 10, 62, 40 } };
    send(sockClient, buffs[0], 35, 0);
    for (int i = 0; i < 10; ++i) {
        buffer[0] = 141;
        buffer[1] = i % 4;
        SEND(2);
        int cnt = 0;
        for (int j = 4; j <= 34; j += 2)
            cnt += buffs[i][j];
        send(sockClient, buffs[i + 1], 35, 0);
        Sleep(cnt * 1000 / 64 + 100);
    }
    buffer[0] = 141;
    buffer[1] = 2;
    SEND(2);
    issing = false;
    return 0;
}

void CALLBACK MainDlg::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    JS->runJoystick(wnd->GetHWND());
    if (JS->getmsg() == WM_TIMER) {
        // add notify
        if (JS->getButton(4) != flg_lbtn) { // start side motor
            flg_lbtn = JS->getButton(4);
            if (flg_lbtn) {
                if (rec_lbtn) {
                    sidemotor = 0;
                    rec_lbtn = 0;
                } else {
                    sidemotor = 127;
                    rec_lbtn = 1;
                }
                buffer[0] = 144;
                buffer[1] = motor;
                buffer[2] = sidemotor;
                buffer[3] = vac;
                SEND(4);
            }
        }
        if (JS->getButton(5) != flg_rbtn) { // clock
            flg_rbtn = JS->getButton(5);
            if (flg_rbtn) {
                sidemotor = -sidemotor;
                buffer[0] = 144;
                buffer[1] = motor;
                buffer[2] = sidemotor;
                buffer[3] = vac;
                SEND(4);
            }
        }
        if (JS->getButton(0) != flg_abtn) {  // motor
            flg_abtn = JS->getButton(0);
            if (flg_abtn) {
                if (rec_abutton) {
                    motor = vac = 0;
                    rec_abutton = 0;
                } else {
                    motor = vac = 127;
                    rec_abutton = 1;
                }
                buffer[0] = 144;
                buffer[1] = motor;
                buffer[2] = sidemotor;
                buffer[3] = vac;
                SEND(4);
            }
        }
        if (JS->getButton(2) != rec_xbutton) {
            if (rec_xbutton) {
                if (!issing) {  // sing
                    issing = true;
                    CreateThread(NULL, 0, Sing, NULL, 0, NULL);
                }
            }
            rec_xbutton = JS->getButton(2);
        }
        if (JS->getButton(1) != rec_bbutton) {
            if (rec_bbutton && !isstart) {
                buffer[0] = 128;    // start
                SEND(1);
                //buffer[0] = 131;    // safe mode
                buffer[0] = 132;    // full mode
                SEND(1);
                isstart = true;
            } else if(rec_bbutton && isstart){
                buffer[0] = 131;
                SEND(1);
                buffer[0] = 173;    // stop
                SEND(1);
                isstart = false;
            }
            rec_bbutton = JS->getButton(1);
        }
        if (JS->getButton(3) != rec_ybutton) {
            if (rec_ybutton) {  // speed step
                step--;
                if (step == 0)
                    step = 3;
            }
            rec_ybutton = JS->getButton(3);
        }
        if (((int)JS->getXAxis() / 100) * 100 != rec_xaxis ||
            ((int)JS->getYAxis() / 100) * 100 != rec_yaxis) {
            rec_xaxis = JS->getXAxis();
            rec_yaxis = JS->getYAxis();
            // fluent control
            bool fhy = rec_yaxis > 0 ? 1 : 0;
            rec_xaxis /= 100; rec_xaxis *= 100;
            rec_yaxis /= 100; rec_yaxis *= 100;
            buffer[0] = 146;    // drive
            if (rec_xaxis == 0 && rec_yaxis == 0) {
                SRIGHT(0);
                SLEFT(0);
            } else if (rec_xaxis == 0) {
                int speed = -OFFSET * rec_yaxis / 1000 * 255;
                SRIGHT(speed);
                SLEFT(speed);
            } else {
                double jiao = std::atan((double)rec_yaxis / rec_xaxis) * 180 / PI;
                if (rec_xaxis > 0 && fhy) {
                    SLEFT(-OFFSET * 255);
                    SRIGHT(int(-OFFSET * jiao / 90 * 255));
                } else if (rec_xaxis < 0 && fhy) {
                    SLEFT(int(OFFSET * jiao / 90 * 255));
                    SRIGHT(-OFFSET * 255);
                } else if (rec_xaxis < 0 && !fhy) {
                    SLEFT(int(OFFSET * jiao / 90 * 255));
                    SRIGHT(OFFSET * 255);
                } else {
                    SLEFT(OFFSET * 255);
                    SRIGHT(int(-OFFSET * jiao / 90 * 255));
                }
            }
            SEND(5);
        }
        if (JS->getPOV0() != rec_pov1) {
            buffer[0] = 146;    // stay drive
            rec_pov1 = JS->getPOV0();
            if (rec_pov1 == -1) {
                SRIGHT(0);
                SLEFT(0);
            } else if (rec_pov1 == 0) {
                SRIGHT(OFFSET * 255);
                SLEFT(OFFSET * 255);
            } else if (rec_pov1 == 9000) {
                SRIGHT(-OFFSET * 255);
                SLEFT(OFFSET * 255);
            } else if (rec_pov1 == 18000) {
                SRIGHT(-OFFSET * 255);
                SLEFT(-OFFSET * 255);
            } else if (rec_pov1 == 27000) {
                SRIGHT(OFFSET * 255);
                SLEFT(-OFFSET * 255);
            }
            SEND(5);
        }
        if (JS->getZAxis() != rec_zaxis) {
            rec_zaxis = JS->getZAxis();
            buffer[0] = 146;
            if (rec_zaxis > 0) {
                SRIGHT(OFFSET * 255);
                SLEFT(-OFFSET * 255);
            } else if (rec_zaxis < 0) {
                SRIGHT(-OFFSET * 255);
                SLEFT(OFFSET * 255);
            } else {
                SRIGHT(0);
                SLEFT(0);
            }
            SEND(5);
        }

        UIText* xaxis = (UIText*)wnd->FindCtrlFromName(_T("txt_xaxis"));
        UIText* yaxis = (UIText*)wnd->FindCtrlFromName(_T("txt_yaxis"));
        UIText* pov1 = (UIText*)wnd->FindCtrlFromName(_T("txt_pov1"));
        UIText* abutton = (UIText*)wnd->FindCtrlFromName(_T("txt_abutton"));
        UIText* xbutton = (UIText*)wnd->FindCtrlFromName(_T("txt_xbutton"));
        UIText* bbutton = (UIText*)wnd->FindCtrlFromName(_T("txt_bbutton"));
        UIText* ybutton = (UIText*)wnd->FindCtrlFromName(_T("txt_ybutton"));
        xaxis->SetText(UStr(rec_xaxis));
        yaxis->SetText(UStr(rec_yaxis));
        pov1->SetText(UStr(rec_pov1));
        abutton->SetText(UStr(rec_abutton));
        xbutton->SetText(UStr(rec_xbutton));
        bbutton->SetText(UStr(rec_bbutton));
        ybutton->SetText(UStr(rec_ybutton));

        wnd->UpdateWindow();
    }

    UIText* txt_state = (UIText*)wnd->FindCtrlFromName(_T("txt_state"));
    wchar_t s[200];
    Str2WStr(JS->getErrorMSG().c_str(), s, 200);
    txt_state->SetText(s);
}
