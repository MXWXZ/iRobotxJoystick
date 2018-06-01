/**
 * Copyright (c) 2018-2050
 * All rights reserved.
 *
 * @Author:MXWXZ
 * @Date:2018/03/20
 */
#pragma once
#include "gdiplus.h"
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")

namespace DuiMini {
class DUIMINI_API UIRenderGDIP :public IUIRender {
public:
    UIRenderGDIP();
    ~UIRenderGDIP();

    bool GlobalInit() override;
    bool GlobalRelease() override;
    bool Paint(UIWindow* v_wnd) override;
    bool RedrawBackground() override;
    bool DrawImage(UIRenderImage* v_img, const UIRect& v_destrect,
                   const UIRect& v_srcrect, ALPHA v_alpha = 255,
                   ImageMode v_mode = kIM_Extrude) override;
    bool DrawString(LPCTSTR v_text, const UIFont &v_font,
                    const UIStringFormat &v_format, const UIRect &v_rect) override;
    bool DrawRect(const UIRect &v_rect, const UIColor &v_color,
                  long v_border) override;
    bool DrawFillRect(const UIRect &v_rect, const UIColor &v_color) override;

private:
    HDC background_ = NULL;         // background DC
    HGDIOBJ tmpobj_ = NULL;         // tmp object
    HBITMAP bg_bitmap_ = NULL;      // bitmap on DC
    Gdiplus::Graphics* graph_ = nullptr;    // always nullptr after used
    static ULONG_PTR gdiplus_token;
};

class DUIMINI_API UIRenderImageGDIP :public IUIRenderImage {
public:
    UIRenderImageGDIP();
    explicit UIRenderImageGDIP(LPCTSTR v_path);
    ~UIRenderImageGDIP();

    bool Load(LPCTSTR v_path) override;
    LPVOID GetInterface() const override;
    long GetWidth() const override;
    long GetHeight() const override;

private:
    shared_ptr<Gdiplus::Image> img_ = nullptr;
};
}    // namespace DuiMini
