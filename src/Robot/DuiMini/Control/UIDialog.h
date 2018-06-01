/**
 * Copyright (c) 2018-2050
 * All rights reserved.
 *
 * @Author:MXWXZ
 * @Date:2018/03/20
 */
#pragma once
#include "Control/UIControl.h"
#include "Control/Layout/UIContainer.h"

namespace DuiMini {
class DUIMINI_API UIDialog :public UIContainer {
public:
    UIDialog();
    virtual ~UIDialog();

public:
    // title
    CUStr GetTitle() const;
    void SetTitle(LPCTSTR v_title);
    /**
     * appwin/movable/resizable
     * @param    BOOL xxx:TRUE/FALSE Allow/Reject, STAY not change state
     * @return   former state
     */
    bool ShowTaskBar(BOOL v_show = TRUE);
    bool AllowWindowMove(BOOL v_movable = TRUE);      // return former state
    bool AllowWindowResize(BOOL v_resizable = TRUE);  // return former state
    // sizebox
    void SetSizeBox(LPCTSTR v_sizestr);
    CUStr GetSizeBox() const;
    // caption
    void SetCaptionRect(LPCTSTR v_pos);
    UIRect GetCaptionRect() const;
    // min/max size
    void SetMinWidth(long v_width);
    long GetMinWidth() const;
    void SetMaxWidth(long v_width);
    long GetMaxWidth() const;
    void SetMinHeight(long v_height);
    long GetMinHeight() const;
    void SetMaxHeight(long v_height);
    long GetMaxHeight() const;

public:
    ATTR_MAP_BEGIN
        DEFAULT_ATTR(_T("title"), _T(""))
        DEFAULT_ATTR(_T("appwin"), _T("1"))
        DEFAULT_ATTR(_T("caption"), _T("0,0,-0,-0"))
        DEFAULT_ATTR(_T("movable"), _T("1"))
        DEFAULT_ATTR(_T("sizebox"), _T("0,0,0,0"))
        DEFAULT_ATTR(_T("resizable"), _T("0"))
        DEFAULT_ATTR(_T("minwidth"), _T("0"))
        DEFAULT_ATTR(_T("maxwidth"), UStr(UIUtils::GetWorkAreaSize().width()))
        DEFAULT_ATTR(_T("minheight"), _T("0"))
        DEFAULT_ATTR(_T("maxheight"), UStr(UIUtils::GetWorkAreaSize().height()))
        PARENT_ATTR(UIContainer)
        ATTR_MAP_END

    LPVOID GetInterface(LPCTSTR v_name) override;
    void AfterSetAttribute() override;
    bool Event(const UIEvent& v_event) override;

protected:
    OVERRIDE_MSGFUNC(OnLangChange)
};
}   // namespace DuiMini
