/**
 * Copyright (c) 2018-2050
 * All rights reserved.
 *
 * @Author:MXWXZ
 * @Date:2018/03/20
 */
#pragma once

namespace DuiMini {
enum RenderMode {
    kRM_GDIPlus,
};

class DUIMINI_API UIRenderImage {
public:
    UIRenderImage();
    explicit UIRenderImage(LPCTSTR v_path);
    ~UIRenderImage();

    bool Load(LPCTSTR v_path);
    void* GetInterface() const;
    long GetWidth() const;
    long GetHeight() const;

private:
    shared_ptr<IUIRenderImage> renderimg_ = nullptr;
};

#define RENDER_CALL(x) if (!render_)             \
                           return false;         \
                       return x

class DUIMINI_API UIRender {
public:
    UIRender();
    ~UIRender();

    /**
     * You ONLY need to call these ONCE at the beginning and the end of the
     * program.
     */
    static bool GlobalInit();
    static bool GlobalInit(RenderMode v_mode);
    // Release will be automatically called by UISystem::Cleanup
    static bool GlobalRelease();

    static void SetRenderMode(RenderMode v_mode);
    static RenderMode GetRenderMode();

    bool Paint(UIWindow* v_wnd);
    bool RedrawBackground();
    bool DrawImage(UIRenderImage* v_img, const UIRect& v_destrect,
                   ALPHA v_alpha = 255, ImageMode v_mode = kIM_Extrude);
    bool DrawImage(UIRenderImage* v_img, const UIRect& v_destrect,
                   const UIRect& v_srcrect, ALPHA v_alpha = 255,
                   ImageMode v_mode = kIM_Extrude);
    bool DrawString(LPCTSTR v_text, const UIFont &v_font,
                    const UIStringFormat &v_format, const UIRect &v_rect);
    bool DrawRect(const UIRect &v_rect, const UIColor &v_color,
                  long v_border);
    bool DrawFillRect(const UIRect &v_rect, const UIColor &v_color);

private:
    static IUIRender* SelectRender(shared_ptr<IUIRender>* v_pointer);

    shared_ptr<IUIRender> render_ = nullptr;
    static RenderMode render_mode_;
};
}    // namespace DuiMini
