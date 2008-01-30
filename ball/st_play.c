/*
 * Copyright (C) 2003 Robert Kooima
 *
 * NEVERBALL is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 2  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include "gui.h"
#include "hud.h"
#include "game.h"
#include "demo.h"
#include "levels.h"
#include "audio.h"
#include "config.h"
#include "st_shared.h"

#include "st_play.h"
#include "st_goal.h"
#include "st_fall_out.h"
#include "st_time_out.h"
#include "st_over.h"
#include "st_pause.h"

/*---------------------------------------------------------------------------*/

static int view_rotate;

static int pause_or_exit(void)
{
    if (SDL_GetModState() & KMOD_SHIFT)
    {
        level_stat(GAME_NONE, curr_clock(), curr_coins());
        level_stop();
        config_clr_grab();

        return goto_state(&st_over);
    }
    return goto_pause();
}

/*---------------------------------------------------------------------------*/

static int play_ready_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Ready?"), GUI_LRG, GUI_ALL, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_READY, 1.0f);
    config_set_grab(1);

    return id;
}

static void play_ready_timer(int id, float dt)
{
    float t = time_state();

    game_set_fly(1.0f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_set);

    game_step_fade(dt);
    gui_timer(id, dt);
}

static int play_ready_click(int b, int d)
{
    return (b < 0 && d == 1) ? goto_state(&st_play_loop) : 1;
}

static int play_ready_keybd(int c, int d)
{
    if (d)
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_pause();
    return 1;
}

static int play_ready_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return pause_or_exit();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int play_set_enter(void)
{
    int id;

    if ((id = gui_label(0, _("Set?"), GUI_LRG, GUI_ALL, 0, 0)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_SET, 1.f);

    clear_pause();

    return id;
}

static void play_set_timer(int id, float dt)
{
    float t = time_state();

    game_set_fly(0.5f - 0.5f * t);

    if (dt > 0.0f && t > 1.0f)
        goto_state(&st_play_loop);

    game_step_fade(dt);
    gui_timer(id, dt);
}

static int play_set_click(int b, int d)
{
    if (b < 0 && d == 1)
    {
        game_set_fly(0.0f);
        return goto_state(&st_play_loop);
    }
    return 1;
}

static int play_set_keybd(int c, int d)
{
    if (d)
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_pause();
    return 1;
}

static int play_set_buttn(int b, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_A, b))
            return goto_state(&st_play_loop);
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            return pause_or_exit();
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static int show_hud;

static int play_loop_enter(void)
{
    int id;

    if (is_paused())
    {
        clear_pause();
        view_rotate = 0;
        return 0;
    }

    if ((id = gui_label(0, _("GO!"), GUI_LRG, GUI_ALL, gui_blu, gui_grn)))
    {
        gui_layout(id, 0, 0);
        gui_pulse(id, 1.2f);
    }

    audio_play(AUD_GO, 1.f);

    game_set_fly(0.f);
    view_rotate = 0;

    hud_view_pulse(config_get_d(CONFIG_CAMERA));

    show_hud = 1;

    hud_update(0);

    return id;
}

static void play_loop_paint(int id, float t)
{
    game_draw(0, t);

    if (show_hud)
        hud_paint();

    if (time_state() < 1.f)
        gui_paint(id);
}

static void play_loop_timer(int id, float dt)
{
    float k = ((SDL_GetModState() & KMOD_SHIFT) ?
               (float) config_get_d(CONFIG_ROTATE_FAST) / 100.0f :
               (float) config_get_d(CONFIG_ROTATE_SLOW) / 100.0f);

    float g[3] = { 0.0f, -9.8f, 0.0f };

    gui_timer(id, dt);
    hud_timer(dt);
    game_set_rot(view_rotate * k);
    game_set_cam(config_get_d(CONFIG_CAMERA));

    game_step_fade(dt);
    demo_play_step();

    switch (game_step(g, dt, 1))
    {
    case GAME_GOAL:
        level_stat(GAME_GOAL, curr_clock(), curr_coins());
        gui_stuck();
        goto_state(&st_goal);
        break;

    case GAME_FALL:
        level_stat(GAME_FALL, curr_clock(), curr_coins());
        gui_stuck();
        goto_state(&st_fall_out);
        break;

    case GAME_TIME:
        level_stat(GAME_TIME, curr_clock(), curr_coins());
        gui_stuck();
        goto_state(&st_time_out);
        break;

    default:
        break;
    }
}

static void play_loop_point(int id, int x, int y, int dx, int dy)
{
    game_set_pos(dx, dy);
}

static void play_loop_stick(int id, int a, int k)
{
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_X, a))
        game_set_z(k);
    if (config_tst_d(CONFIG_JOYSTICK_AXIS_Y, a))
        game_set_x(k);
}

static int play_loop_click(int b, int d)
{
    view_rotate = d ? b : 0;
    return 1;
}

static int play_loop_keybd(int c, int d)
{
    if (d)
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            view_rotate = +1;
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            view_rotate = -1;

        if (config_tst_d(CONFIG_KEY_CAMERA_1, c))
        {
            config_set_d(CONFIG_CAMERA, 0);
            hud_view_pulse(0);
        }
        if (config_tst_d(CONFIG_KEY_CAMERA_2, c))
        {
            config_set_d(CONFIG_CAMERA, 1);
            hud_view_pulse(1);
        }
        if (config_tst_d(CONFIG_KEY_CAMERA_3, c))
        {
            config_set_d(CONFIG_CAMERA, 2);
            hud_view_pulse(2);
        }
        if (config_tst_d(CONFIG_KEY_RESTART, c) &&
            curr_lg()->mode != MODE_CHALLENGE)
        {
            level_same();
            goto_state(&st_play_ready);
        }
        if (config_tst_d(CONFIG_KEY_PAUSE, c))
            goto_pause();
    }
    else
    {
        if (config_tst_d(CONFIG_KEY_CAMERA_R, c))
            view_rotate = 0;
        if (config_tst_d(CONFIG_KEY_CAMERA_L, c))
            view_rotate = 0;
    }

    if (d && c == SDLK_F12 && config_cheat())
        return goto_state(&st_look);

    if (d && c == SDLK_F6)
        show_hud = !show_hud;

    if (d && c == SDLK_c && config_cheat())
    {
        level_stat(GAME_GOAL, curr_clock(), curr_coins());
        return goto_state(&st_goal);
    }
    return 1;
}

static int play_loop_buttn(int b, int d)
{
    if (d == 1)
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
            pause_or_exit();

        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = +1;
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = -1;

        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_1, b))
        {
            config_set_d(CONFIG_CAMERA, 0);
            hud_view_pulse(0);
        }
        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_2, b))
        {
            config_set_d(CONFIG_CAMERA, 1);
            hud_view_pulse(1);
        }
        if (config_tst_d(CONFIG_JOYSTICK_CAMERA_3, b))
        {
            config_set_d(CONFIG_CAMERA, 2);
            hud_view_pulse(2);
        }
    }
    else
    {
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_R, b))
            view_rotate = 0;
        if (config_tst_d(CONFIG_JOYSTICK_BUTTON_L, b))
            view_rotate = 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static float phi;
static float theta;

static int look_enter(void)
{
    phi   = 0;
    theta = 0;
    return 0;
}

static void look_leave(int id)
{
}

static void look_paint(int id, float t)
{
    game_draw(0, t);
}

static void look_point(int id, int x, int y, int dx, int dy)
{
    phi   +=  90.0f * dy / config_get_d(CONFIG_HEIGHT);
    theta += 180.0f * dx / config_get_d(CONFIG_WIDTH);

    if (phi > +90.0f) phi = +90.0f;
    if (phi < -90.0f) phi = -90.0f;

    if (theta > +180.0f) theta -= 360.0f;
    if (theta < -180.0f) theta += 360.0f;

    game_look(phi, theta);
}

static int look_keybd(int c, int d)
{
    if (d && c == SDLK_F12)
        return goto_state(&st_play_loop);

    return 1;
}

static int look_buttn(int b, int d)
{
    if (d && config_tst_d(CONFIG_JOYSTICK_BUTTON_EXIT, b))
        return goto_state(&st_play_loop);

    return 1;
}

/*---------------------------------------------------------------------------*/

struct state st_play_ready = {
    play_ready_enter,
    shared_leave,
    shared_paint,
    play_ready_timer,
    NULL,
    NULL,
    NULL,
    play_ready_click,
    play_ready_keybd,
    play_ready_buttn,
    1, 0
};

struct state st_play_set = {
    play_set_enter,
    shared_leave,
    shared_paint,
    play_set_timer,
    NULL,
    NULL,
    NULL,
    play_set_click,
    play_set_keybd,
    play_set_buttn,
    1, 0
};

struct state st_play_loop = {
    play_loop_enter,
    shared_leave,
    play_loop_paint,
    play_loop_timer,
    play_loop_point,
    play_loop_stick,
    shared_angle,
    play_loop_click,
    play_loop_keybd,
    play_loop_buttn,
    0, 0
};

struct state st_look = {
    look_enter,
    look_leave,
    look_paint,
    NULL,
    look_point,
    NULL,
    NULL,
    NULL,
    look_keybd,
    look_buttn,
    0, 0
};