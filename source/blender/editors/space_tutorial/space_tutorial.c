/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2017 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup sptutorial
 */

#include <string.h>
#include <stdio.h>

#include "MEM_guardedalloc.h"

#include "DNA_text_types.h"

#include "BLI_blenlib.h"

#include "BKE_context.h"
#include "BKE_screen.h"
#include "BLI_math.h"
//#include "BIF_gl.h"

#include "ED_space_api.h"
#include "ED_screen.h"
#include "ED_transform.h"

#include "UI_interface.h"
#include "UI_resources.h"
#include "UI_view2d.h"

#include "WM_api.h"
#include "WM_types.h"

#include "GPU_immediate.h"
#include "GPU_immediate_util.h"
#include "GPU_matrix.h"
#include "GPU_state.h"
#include "GPU_framebuffer.h"

#include "../interface/interface_intern.h" // make note of this!!!


//#define DEBUG
enum { RED = 1, GREEN, BLUE };

int turn_background_red_in_main_region(struct bContext *C, struct wmOperator *oper) {
    printf("SPACE_TUTORIAL_OT_red_region called!\n");

    SpaceTutorial* stut = CTX_wm_space_tutorial(C);
    ARegion *ar = CTX_wm_region(C);

    stut->color = 1;

    ED_region_tag_redraw(ar);
    ED_area_tag_redraw(CTX_wm_area(C));

    return OPERATOR_FINISHED;
}

int turn_background_green_in_main_region(struct bContext *C, struct wmOperator *oper) {
    printf("SPACE_TUTORIAL_OT_green_region called!\n");

    SpaceTutorial* stut = CTX_wm_space_tutorial(C);
    ARegion *ar = CTX_wm_region(C);

    stut->color = 2;

    ED_region_tag_redraw(ar);
    ED_area_tag_redraw(CTX_wm_area(C));

    return OPERATOR_FINISHED;
}

int turn_background_blue_in_main_region(struct bContext *C, struct wmOperator *oper) {
    printf("SPACE_TUTORIAL_OT_blue_region called!\n");

    SpaceTutorial* stut = CTX_wm_space_tutorial(C);
    ARegion *ar = CTX_wm_region(C);

    stut->color = 3;

    ED_region_tag_redraw(ar);
    ED_area_tag_redraw(CTX_wm_area(C));

    return OPERATOR_FINISHED;
}

int test_context_for_button_operator(struct bContext* C) {
    return 1;
}

void SPACE_TUTORIAL_OT_red_region(wmOperatorType *ot)
{
    /* identifiers */
    ot->name = "Red Region Button";
    ot->description = "Turns the main region background to red";
    ot->idname = "SPACE_TUTORIAL_OT_red_region";

    /* api callbacks */
    ot->exec = turn_background_red_in_main_region;
    ot->poll = test_context_for_button_operator;//ED_operator_region_view3d_active;

    /* flags */
    ot->flag = OPTYPE_REGISTER;
}

void SPACE_TUTORIAL_OT_green_region(wmOperatorType *ot)
{
    /* identifiers */
    ot->name = "Green Region Button";
    ot->description = "Turns the main region background to green";
    ot->idname = "SPACE_TUTORIAL_OT_green_region";

    /* api callbacks */
    ot->exec = turn_background_green_in_main_region;
    ot->poll = test_context_for_button_operator;//ED_operator_region_view3d_active;

    /* flags */
    ot->flag = OPTYPE_REGISTER;
}

void SPACE_TUTORIAL_OT_blue_region(wmOperatorType *ot)
{
    /* identifiers */
    ot->name = "Blue Region Button";
    ot->description = "Turns the main region background to blue";
    ot->idname = "SPACE_TUTORIAL_OT_blue_region";

    /* api callbacks */
    ot->exec = turn_background_blue_in_main_region;
    ot->poll = test_context_for_button_operator;//ED_operator_region_view3d_active;

    /* flags */
    ot->flag = OPTYPE_REGISTER;
}

void tutorial_operatortypes(void)
{
    WM_operatortype_append(SPACE_TUTORIAL_OT_red_region);
    WM_operatortype_append(SPACE_TUTORIAL_OT_green_region);
    WM_operatortype_append(SPACE_TUTORIAL_OT_blue_region);
}

static void tutorial_init(struct wmWindowManager *wm, struct ScrArea *sa) {
    SpaceTutorial *stut = (SpaceTutorial *)sa->spacedata.first;
//  stut->color = GREEN;
}


static SpaceLink *tutorial_new(const ScrArea *UNUSED(area), const Scene *UNUSED(scene))
{
    ARegion *ar;
    SpaceTutorial *stutorial;

    stutorial = MEM_callocN(sizeof(*stutorial), "init tutorial");
    stutorial->spacetype = SPACE_TUTORIAL;

    // Data-blocks init...

    /* header */
    ar = MEM_callocN(sizeof(ARegion), "header for tutorial");

    BLI_addtail(&stutorial->regionbase, ar);
    ar->regiontype = RGN_TYPE_HEADER;
    ar->alignment = RGN_ALIGN_BOTTOM;

    /* main regions */
    ar = MEM_callocN(sizeof(ARegion), "main region of tutorial");

    BLI_addtail(&stutorial->regionbase, ar);
    ar->regiontype = RGN_TYPE_WINDOW;

    /* ui region */
    ar = MEM_callocN(sizeof(ARegion), "ui region of tutorial");

    BLI_addtail(&stutorial->regionbase, ar);
    ar->regiontype = RGN_TYPE_UI;

    return (SpaceLink *)stutorial;
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_main_region_init(wmWindowManager *wm, ARegion *region)
{
    UI_view2d_region_reinit(&region->v2d, V2D_COMMONVIEW_CUSTOM, region->winx, region->winy);
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_tools_region_init(wmWindowManager *wm, ARegion *region)
{
    UI_view2d_region_reinit(&region->v2d, V2D_COMMONVIEW_CUSTOM, region->winx, region->winy);
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_main_region_draw(const bContext *C, ARegion *ar)
{
    printf("in tutorial_main_region_draw\n");

    SpaceTutorial* stut = CTX_wm_space_tutorial(C);
    View2D *v2d = &ar->v2d;

    switch(stut->color) {
    case GREEN:
        GPU_clear_color(0.0, 1.0, 0.0, 1.0);
        break;
    case BLUE:
        GPU_clear_color(0.0, 0.0, 1.0, 1.0);
        break;
    case RED:
    default:
        GPU_clear_color(1.0, 0.0, 0.0, 1.0);
    }

    GPU_clear(GPU_COLOR_BIT);

    /* draw colored rectangles within mask area of region */
#if DEBUG
    printf("v2d->mask.xmin == %d\n", v2d->mask.xmin);
    printf("v2d->mask.ymin == %d\n", v2d->mask.ymin);
    printf("v2d->mask.xmax == %d\n", v2d->mask.xmax);
    printf("v2d->mask.ymax == %d\n", v2d->mask.ymax);
#endif

    uint pos = GPU_vertformat_attr_add(
                   immVertexFormat(), "pos", GPU_COMP_I32, 2, GPU_FETCH_INT_TO_FLOAT);
    immBindBuiltinProgram(GPU_SHADER_2D_UNIFORM_COLOR);

    immUniformColor4ub(255, 0, 255, 255);
    immRecti(pos,
             v2d->mask.xmin + 50,
             v2d->mask.ymin + 50,
             v2d->mask.xmax - 50,
             v2d->mask.ymax - 50);

    immUniformColor4ub(0, 255, 255, 255);
    immRecti(pos,
             v2d->mask.xmin + 80,
             v2d->mask.ymin + 80,
             v2d->mask.xmax - 80,
             v2d->mask.ymax - 80);

    immUniformColor4ub(255, 255, 0, 255);
    immRecti(pos,
             v2d->mask.xmin + 110,
             v2d->mask.ymin + 110,
             v2d->mask.xmax - 110,
             v2d->mask.ymax - 110);

    immUnbindProgram();
}



/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_header_region_init(wmWindowManager *UNUSED(wm), ARegion *ar)
{
    ED_region_header_init(ar);
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_ui_region_init(wmWindowManager *wm, ARegion *ar)
{
    ED_region_panels_init(wm, ar);
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_ui_region_draw(const bContext *C, ARegion *ar)
{
    /* takes us to the Python API code for panels */
    printf("in tutorial_ui_region_draw\n");
//  ED_region_panels_draw(C, ar);
}

static void draw_our_GUI_in_C(uiBlock *block, uiLayout *layout) {
    static int val = 100;

    struct wmOperatorType* ot = WM_operatortype_find("SPACE_TUTORIAL_OT_red_region", true);

    struct uiBut* but = uiDefBut(block,            // uiBlock *block
                                 UI_BTYPE_BUT_TOGGLE,  // int type
                                 1,                // int retval
                                 "RED",          // const char *str
                                 100,                // int x1
                                 2,                // int y1
                                 30,               // short x2
                                 19,               // short y2
                                 (void*)&val,      // void *poin to char, int, float ?
                                 0.0,                // float min
                                 0.0,               // float max
                                 0.0,                // float a1
                                 0.0,                // float a2
                                 "");              // const char *tip
    but->optype = ot;

    ot = WM_operatortype_find("SPACE_TUTORIAL_OT_green_region", true);
    but = uiDefBut(block,            // uiBlock *block
                   UI_BTYPE_BUT_TOGGLE,  // int type
                   2,                // int retval
                   "GREEN",          // const char *str
                   200,                // int x1
                   5,                // int y1
                   75,               // short x2
                   19,               // short y2
                   (void*)&val,      // void *poin to char, int, float ?
                   0.0,                // float min
                   0.0,               // float max
                   0.0,                // float a1
                   0.0,                // float a2
                   "");              // const char *tip
    but->optype = ot;

    ot = WM_operatortype_find("SPACE_TUTORIAL_OT_blue_region", true);
    but = uiDefBut(block,            // uiBlock *block
                   UI_BTYPE_BUT_TOGGLE,  // int type
                   3,                // int retval
                   "BLUE",          // const char *str
                   300,                // int x1
                   5,                // int y1
                   175,               // short x2
                   19,               // short y2
                   (void*)&val,      // void *poin to char, int, float ?
                   0.0,                // float min
                   0.0,               // float max
                   0.0,                // float a1
                   0.0,                // float a2
                   "");              // const char *tip
    but->optype = ot;
}

/* add handlers, stuff you only do once or on area/region changes */
static void tutorial_header_region_draw(const bContext *C, ARegion *ar)
{

    // taken from ED_region_layout...
    uiStyle *style = UI_style_get_dpi();
    uiBlock *block;
    uiLayout *layout;
    bool region_layout_based = ar->flag & RGN_FLAG_DYNAMIC_SIZE;

    /* Height of buttons and scaling needed to achieve it. */
    const int buttony = min_ii(UI_UNIT_Y, ar->winy - 2 * UI_DPI_FAC);
    const float buttony_scale = buttony / (float)UI_UNIT_Y;

    /* Vertically center buttons. */
    int xco = UI_HEADER_OFFSET;
    int yco = buttony + (ar->winy - buttony) / 2;
    int maxco = xco;

    /* set view2d view matrix for scrolling (without scrollers) */
    UI_view2d_view_ortho(&ar->v2d);

    block = UI_block_begin(C, ar, "", UI_EMBOSS);
    layout = UI_block_layout(
                 block, UI_LAYOUT_HORIZONTAL, UI_LAYOUT_HEADER, xco, yco, buttony, 1, 0, style);

    if (buttony_scale != 1.0f) {
        uiLayoutSetScaleY(layout, buttony_scale);
    }

    /* our GUI in C */
    draw_our_GUI_in_C(block, layout);

    UI_block_layout_resolve(block, &xco, &yco);

    /* for view2d */
    if (xco > maxco) {
        maxco = xco;
    }

    int new_sizex = (maxco + UI_HEADER_OFFSET) / UI_DPI_FAC;

    if (region_layout_based && (ar->sizex != new_sizex)) {
        /* region size is layout based and needs to be updated */
        ScrArea *sa = CTX_wm_area(C);

        ar->sizex = new_sizex;
        sa->flag |= AREA_FLAG_REGION_SIZE_UPDATE;
    }

    UI_block_end(C, block);

    if (!region_layout_based) {
        maxco += UI_HEADER_OFFSET;
    }

    /* always as last  */
    UI_view2d_totRect_set(&ar->v2d, maxco, ar->winy);

    /* restore view matrix */
    UI_view2d_view_restore(C);

    /* clear */
    UI_ThemeClearColor(TH_HEADER);
    GPU_clear(GPU_COLOR_BIT);

    UI_view2d_view_ortho(&ar->v2d);

    /* View2D matrix might have changed due to dynamic sized regions. */
    UI_blocklist_update_window_matrix(C, &ar->uiblocks);

    /* draw blocks */
    UI_blocklist_draw(C, &ar->uiblocks);

    /* restore view matrix */
    UI_view2d_view_restore(C);
}

/* only called once, from space/spacetypes.c */
void ED_spacetype_tutorial(void)
{
    SpaceType *st = MEM_callocN(sizeof(SpaceType), "spacetype tutorial");
    ARegionType *art;

    st->spaceid = SPACE_TUTORIAL;
    strncpy(st->name, "Tutorial", BKE_ST_MAXNAME);

    st->new = tutorial_new;
    st->init = tutorial_init;
    st->operatortypes = tutorial_operatortypes;

    /* regions: main window */
    art = MEM_callocN(sizeof(ARegionType), "spacetype tutorial main region");
    art->regionid = RGN_TYPE_WINDOW;
    art->init = tutorial_main_region_init;
    art->draw = tutorial_main_region_draw;

    BLI_addhead(&st->regiontypes, art);

    /* regions: header */
    art = MEM_callocN(sizeof(ARegionType), "spacetype tutorial header region");
    art->regionid = RGN_TYPE_HEADER;
    art->prefsizey = HEADERY;
    art->keymapflag = ED_KEYMAP_UI | ED_KEYMAP_VIEW2D | ED_KEYMAP_HEADER;
    art->init = tutorial_header_region_init;
    art->draw = tutorial_header_region_draw;
    art->prefsizey = HEADERY;

    BLI_addhead(&st->regiontypes, art);

    /* regions: header */
    art = MEM_callocN(sizeof(ARegionType), "spacetype tutorial buttons region");
    art->regionid = RGN_TYPE_UI;
    art->keymapflag = ED_KEYMAP_UI | ED_KEYMAP_VIEW2D | ED_KEYMAP_HEADER;
    art->init = tutorial_ui_region_init;
    art->draw = tutorial_ui_region_draw;
    art->prefsizex = UI_SIDEBAR_PANEL_WIDTH;
    art->keymapflag = ED_KEYMAP_UI | ED_KEYMAP_FRAMES;

    BLI_addhead(&st->regiontypes, art);

    BKE_spacetype_register(st);
}
