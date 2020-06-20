#include <common/BuildConfig.h>
#include <patch_common/CallHook.h>
#include <patch_common/CodeInjection.h>
#include <d3d8.h>

extern HWND g_editor_wnd;

namespace red
{
    struct GrScreen
    {
        int signature;
        int max_width;
        int max_height;
        int mode;
        int window_mode;
        int field_14;
        float aspect;
        int field_1c;
        int bits_per_pixel;
        int bytes_ber_pixel;
        int field_28;
        int offset_x;
        int offset_y;
        int clip_width;
        int clip_height;
        int max_tex_width;
        int max_tex_height;
        int clip_left;
        int clip_right;
        int clip_top;
        int clip_bottom;
        int current_color;
        int current_bitmap;
        int current_bitmap2;
        int fog_mode;
        int fog_color;
        float fog_near;
        float fog_far;
        float fog_far_scalled;
        bool recolor_enabled;
        float recolor_red;
        float recolor_green;
        float recolor_blue;
        int field_84;
        int field_88;
        int zbuffer_mode;
    };
    static_assert(sizeof(GrScreen) == 0x90);

    auto& gr_d3d_buffers_locked = AddrAsRef<bool>(0x0183930D);
    auto& gr_d3d_primitive_type = AddrAsRef<D3DPRIMITIVETYPE>(0x0175A2C8);
    auto& gr_d3d_max_hw_vertex = AddrAsRef<int>(0x01621FA8);
    auto& gr_d3d_max_hw_index = AddrAsRef<int>(0x01621FAC);
    auto& gr_d3d_num_vertices = AddrAsRef<int>(0x01839310);
    auto& gr_d3d_num_indices = AddrAsRef<int>(0x01839314);
    auto& gr_screen = AddrAsRef<GrScreen>(0x014CF748);

}

CallHook<void()> frametime_update_hook{
    0x00483047,
    []() {
        if (!IsWindowVisible(g_editor_wnd)) {
            // when minimized 1 FPS
            Sleep(1000);
        }
        else if (GetForegroundWindow() != g_editor_wnd) {
            // when in background 4 FPS
            Sleep(250);
        }
        frametime_update_hook.CallTarget();
    },
};

CodeInjection after_gr_init_hook{
    0x004B8D4D,
    []() {
        // Fix performance issues caused by this field being initialized to inf
        xlog::info("red::gr_screen.fog_far_scalled %f", red::gr_screen.fog_far_scalled);
        red::gr_screen.fog_far_scalled = 255.0f / red::gr_screen.fog_far;
    },
};

CodeInjection gr_d3d_draw_line_3d_patch_1{
    0x004E133E,
    [](auto& regs) {
        bool flush_needed = !red::gr_d3d_buffers_locked
                         || red::gr_d3d_primitive_type != D3DPT_LINELIST
                         || red::gr_d3d_max_hw_vertex + 2 > 6000
                         || red::gr_d3d_max_hw_index + red::gr_d3d_num_indices + 2 > 10000;
        if (!flush_needed) {
            xlog::trace("Skipping gr_d3d_prepare_buffers");
            regs.eip = 0x004E1343;
        }
        else {
            xlog::trace("Line drawing requires gr_d3d_prepare_buffers %d %d %d %d",
                 red::gr_d3d_buffers_locked, red::gr_d3d_primitive_type, red::gr_d3d_max_hw_vertex,
                 red::gr_d3d_max_hw_index + red::gr_d3d_num_indices);
        }
    },
};

CallHook<void()> gr_d3d_draw_line_3d_patch_2{
    0x004E1528,
    []() {
        red::gr_d3d_num_vertices += 2;
    },
};

CodeInjection gr_d3d_draw_line_2d_patch_1{
    0x004E10BD,
    [](auto& regs) {
        bool flush_needed = !red::gr_d3d_buffers_locked
                         || red::gr_d3d_primitive_type != D3DPT_LINELIST
                         || red::gr_d3d_max_hw_vertex + 2 > 6000
                         || red::gr_d3d_max_hw_index + red::gr_d3d_num_indices + 2 > 10000;
        if (!flush_needed) {
            xlog::trace("Skipping gr_d3d_prepare_buffers");
            regs.eip = 0x004E10C2;
        }
        else {
            xlog::trace("Line drawing requires gr_d3d_prepare_buffers %d %d %d %d",
                 red::gr_d3d_buffers_locked, red::gr_d3d_primitive_type, red::gr_d3d_max_hw_vertex,
                 red::gr_d3d_max_hw_index + red::gr_d3d_num_indices);
        }
    },
};

CallHook<void()> gr_d3d_draw_line_2d_patch_2{
    0x004E11F2,
    []() {
        red::gr_d3d_num_vertices += 2;
    },
};

CodeInjection gr_d3d_draw_poly_patch{
    0x004E1573,
    [](auto& regs) {
        if (red::gr_d3d_primitive_type != D3DPT_TRIANGLELIST) {
            regs.eip = 0x004E1598;
        }
    },
};

CodeInjection gr_d3d_draw_texture_patch_1{
    0x004E090E,
    [](auto& regs) {
        if (red::gr_d3d_primitive_type != D3DPT_TRIANGLELIST) {
            regs.eip = 0x004E092C;
        }
    },
};

CodeInjection gr_d3d_draw_texture_patch_2{
    0x004E0C97,
    [](auto& regs) {
        if (red::gr_d3d_primitive_type != D3DPT_TRIANGLELIST) {
            regs.eip = 0x004E0CBB;
        }
    },
};

CodeInjection gr_d3d_draw_geometry_face_patch_1{
    0x004E18F1,
    [](auto& regs) {
        if (red::gr_d3d_primitive_type != D3DPT_TRIANGLELIST) {
            regs.eip = 0x004E1916;
        }
    },
};

CodeInjection gr_d3d_draw_geometry_face_patch_2{
    0x004E1B2D,
    [](auto& regs) {
        if (red::gr_d3d_primitive_type != D3DPT_TRIANGLELIST) {
            regs.eip = 0x004E1B53;
        }
    },
};

void ApplyGraphicsPatches()
{
#if D3D_HW_VERTEX_PROCESSING
    // Use hardware vertex processing instead of software processing
    WriteMem<u8>(0x004EC73E + 1, D3DCREATE_HARDWARE_VERTEXPROCESSING);
    WriteMem<u32>(0x004EBC3D + 1, D3DUSAGE_DYNAMIC|D3DUSAGE_DONOTCLIP|D3DUSAGE_WRITEONLY);
    WriteMem<u32>(0x004EBC77 + 1, D3DUSAGE_DYNAMIC|D3DUSAGE_DONOTCLIP|D3DUSAGE_WRITEONLY);
#endif

    // Avoid flushing D3D buffers in GrSetColor
    AsmWriter(0x004B976D).nop(5);

    // Add Sleep if window is inactive
    frametime_update_hook.Install();

    // Improve performance
    after_gr_init_hook.Install();

    // Reduce number of draw-calls for line rendering
    AsmWriter(0x004E1335).nop(5);
    gr_d3d_draw_line_3d_patch_1.Install();
    gr_d3d_draw_line_3d_patch_2.Install();
    AsmWriter(0x004E10B4).nop(5);
    gr_d3d_draw_line_2d_patch_1.Install();
    gr_d3d_draw_line_2d_patch_2.Install();
    gr_d3d_draw_poly_patch.Install();
    gr_d3d_draw_texture_patch_1.Install();
    gr_d3d_draw_texture_patch_2.Install();
    gr_d3d_draw_geometry_face_patch_1.Install();
    gr_d3d_draw_geometry_face_patch_2.Install();
}
