#pragma once

#include "common.h"
#include "graphics.h"
#include <patch_common/MemUtils.h>

namespace rf
{
    struct UiGadget
    {
        void **vtbl;
        UiGadget *parent;
        bool highlighted;
        bool enabled;
        int x;
        int y;
        int w;
        int h;
        int key;
        void(*on_click)();
        void(*on_mouse_btn_down)();

        int GetAbsoluteX() const
        {
            return AddrCaller{0x004569E0}.this_call<int>(this);
        }

        int GetAbsoluteY() const
        {
            return AddrCaller{0x00456A00}.this_call<int>(this);
        }
    };
    static_assert(sizeof(UiGadget) == 0x28);

    struct UiButton : UiGadget
    {
        int bg_bitmap;
        int selected_bitmap;
#ifdef DASH_FACTION
        int reserved0;
        char* text;
        int font;
        int reserved1[2];
#else
        int text_user_bmap;
        int text_offset_x;
        int text_offset_y;
        int text_width;
        int text_height;
#endif
    };
    static_assert(sizeof(UiButton) == 0x44);

    struct UiLabel : UiGadget
    {
        Color clr;
        int bitmap;
#ifdef DASH_FACTION
        char* text;
        int font;
        rf::GrTextAlignment align;
#else
        int text_offset_x;
        int text_offset_y;
        int text_width;
#endif
        int text_height;
    };
    static_assert(sizeof(UiLabel) == 0x40);

    struct UiInputBox : UiLabel
    {
        char text[32];
        int max_text_width;
        int font;
    };
    static_assert(sizeof(UiInputBox) == 0x68);

    struct UiCycler : UiGadget
    {
        static constexpr int max_items = 16;
        int item_text_bitmaps[max_items];
#ifdef DASH_FACTION
        char* items_text[max_items];
        int items_font[max_items];
#else
        int item_text_offset_x[max_items];
        int item_text_offset_y[max_items];
#endif
        int items_width[max_items];
        int items_height[max_items];
        int num_items;
        int current_item;
    };
    static_assert(sizeof(UiCycler) == 0x170);

    static auto& UiMsgBox = AddrAsRef<void(const char *title, const char *text, void(*callback)(), bool input)>(0x004560B0);
    using UiDialogCallbackPtr = void (*)();
    static auto& UiCreateDialog =
        AddrAsRef<void(const char *title, const char *text, unsigned num_buttons, const char *ppsz_btn_titles[],
                       UiDialogCallbackPtr callbacks[], unsigned unknown1, unsigned unknown2)>(0x004562A0);
    static auto& UiGetGadgetFromPos = AddrAsRef<int(int x, int y, UiGadget * const gadgets[], int num_gadgets)>(0x00442ED0);
    static auto& UiUpdateInputBoxCursor = AddrAsRef<void()>(0x00456960);

    static auto& ui_scale_x = AddrAsRef<float>(0x00598FB8);
    static auto& ui_scale_y = AddrAsRef<float>(0x00598FBC);
    static auto& ui_input_box_cursor_visible = AddrAsRef<bool>(0x00642DC8);
    static auto& ui_large_font = AddrAsRef<int>(0x0063C05C);
    static auto& ui_medium_font_0 = AddrAsRef<int>(0x0063C060);
    static auto& ui_medium_font_1 = AddrAsRef<int>(0x0063C064);
    static auto& ui_small_font = AddrAsRef<int>(0x0063C068);
}
