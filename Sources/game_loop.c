//
// Created by Valentin on 10/6/2019.
//

#ifdef _WIN32
#include "../Header/curses.h"
#else
#include <ncurses.h>
#endif

#include <stdlib.h>
#include <time.h>
#include "../Header/core_game.h"

static bool was_initialized = false;

/// DEBUG WINDOW. MIGHT GET REPLACED BY THE PARENT WINDOW LATER ON
extern WINDOW *debug;

static void create_monster_ptr(core_game_t *core)
{
    // TODO monster should be passive until player are x blocks away
    srand(time(NULL));
    core->monster_arr = malloc(sizeof(charac_t) * core->size_monster_arr);
    for (int i = 0; i < core->size_monster_arr; i++) {
        core->monster_arr[i].stat = (stat_t) {.experience = -1, .level = 1, .health = 10};
        core->monster_arr[i].p_cursor.x = rand() % core->floors[core->current_stage].c_room.width;
        core->monster_arr[i].p_cursor.y = rand() % core->floors[core->current_stage].c_room.height;
        if (core->monster_arr[i].p_cursor.x == core->player.p_cursor.x)
            core->monster_arr[i].p_cursor.x = rand() % core->floors[core->current_stage].c_room.width;
        if (core->monster_arr[i].p_cursor.y == core->player.p_cursor.y)
            core->monster_arr[i].p_cursor.y = rand() % core->floors[core->current_stage].c_room.height;
        core->monster_arr[i].repr = 'M';
        core->monster_arr[i].stat.state = alive;
        core->monster_arr[i].is_weapon_dual_hand = true;
        core->monster_arr[i].right_hand = bare_hand;
        core->monster_arr[i].left_hand = bare_hand;
    }
    was_initialized = true;
}

static bool cell_occupied(core_game_t *core, const int index, const point_t current, int *key)
{
    /// KEY IS ONLY USED WHEN A MONSTER IS BEING CHECKED. OTHERWISE, USE KEY > ARR_SIZE
    for (unsigned int i = 0; i < core->size_monster_arr; i++) {
        if (i != index) {
            if (core->monster_arr[i].p_cursor.x == current.x &&
            core->monster_arr[i].p_cursor.y == current.y) {
                if (key != NULL) {
                    *key = i;
                }
                return true;
            }
        }
    }
    if (index == core->size_monster_arr) {
        if (current.x == core->floors[core->current_stage].stairs.cursor.x && current.y == core->floors[core->current_stage].stairs.cursor.y) {
            *key = core->size_monster_arr;
            return true;
        }
    }
    return false;
}

static bool point_equals(point_t first, point_t second)
{
    if (first.x == second.x && first.y == second.y) {
        return true;
    }
    return false;
}

static void movement_quality_check(point_t *current_mob, point_t blocking_point, core_game_t *core)
{
    int dx = blocking_point.x - current_mob->x;
    int dy = blocking_point.y - current_mob->y;
    int choice = (int) rand() % 2;

    wclear(core->logs.logs);
    wprintw(core->logs.logs, "%d\t%d", dx, dy);
    wrefresh(core->logs.logs);
    if (dx == dy && dx == 0) {
        return;
    }
    if (dx != 0) {
        if (current_mob->y > 0 && current_mob->y < (int) core->floors->c_room.height - 1) {
            current_mob->y = current_mob->y + ((choice == 0) ? 1 : -1);
        } else if (current_mob->y == 0) {
            current_mob->y++;
        } else {
            current_mob->y--;
        }
    } else if (dy != 0) {
        if (current_mob->x > 0 && current_mob->x < (int) core->floors->c_room.width - 1) {
            current_mob->x = current_mob->x + ((choice == 0) ? 1 : -1);
        } else if (current_mob->x == 0) {
            current_mob->x++;
        } else {
            current_mob->x--;
        }
    }
}

static void update_path_monster(core_game_t *core)
{
    point_t old_tmp;

    for (unsigned int i = 0; i < core->size_monster_arr; i++) {
        if (core->monster_arr[i].stat.state == true) {
            old_tmp = core->monster_arr[i].p_cursor;
            core->monster_arr[i].p_cursor = search_next_cell(core->monster_arr[i].p_cursor, core->player.p_cursor,
                                                             core->floors[core->current_stage].c_room);
            if (point_equals(core->monster_arr[i].p_cursor, core->player.p_cursor)) {
                core->monster_arr[i].p_cursor = old_tmp;
                set_attack(&core->monster_arr[i], &core->player, core);
            }
            if (cell_occupied(core, (int) i, core->monster_arr[i].p_cursor, NULL) == true) {
                movement_quality_check(&old_tmp, core->monster_arr[i].p_cursor, core);
                core->monster_arr[i].p_cursor = old_tmp;
            }
        }
    }
    move_monster(core);
}

void main_loop(core_game_t *core, int key)
{
    point_t old_p_cursor;
    int key_arr = 0;

    if (was_initialized == false) {
        core->size_monster_arr = 10;
        create_monster_ptr(core);
    }
    old_p_cursor = core->player.p_cursor;
    input_treat(key, core);
    core->player.p_cursor = verify_player_position(core->player.p_cursor, core->floors[core->current_stage].c_room);
    if (cell_occupied(core, (int)core->size_monster_arr, core->player.p_cursor, &key_arr) == true) {
        if (key_arr == core->size_monster_arr) {
            // changement d'étage
            wmove(core->logs.logs, 0, 0);
            wprintw(core->logs.logs, "Press SPACE to continue\n");
            wrefresh(core->logs.logs);
            key = wgetch(core->game_screen);
            if (key == ' ') {
                core->current_stage++;
                core->floors[1].c_room = initialize_room(1000, 1000);
                wclear(core->game_screen);
                print_room(core);
                return;
            }
        } else {
            core->player.p_cursor = old_p_cursor;
            set_attack(&core->player, &core->monster_arr[key_arr], core);
        }
     }
    print_room(core);
    update_path_monster(core);
    print_stats(core->game_screen, core->player);
    //print_logs(core);
}