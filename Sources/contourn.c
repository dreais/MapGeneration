//
// Created by Valentin on 9/25/2019.
//

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h> // will leave it, i often use printf here
#include "../Header/shape.h"

static const int directions[4][2] = {
        {0, -1}, // TOP
        {1, 0}, // RIGHT
        {0, 1}, // BOTTOM
        {-1, 0}}; // LEFT

typedef enum {TOP, RIGHT, BOTTOM, LEFT} dir_name;

static void insert_enum(dir_name *order, const dir_name to_insert[4])
{
    order[0] = to_insert[0];
    order[1] = to_insert[1];
    order[2] = to_insert[2];
    order[3] = to_insert[3];
}

static point_t *append_point(point_t *pool, const unsigned int size, const point_t to_append)
{
    point_t *new = calloc(size + 1, sizeof(point_t));

    for (unsigned int i = 0; i < size; i++) {
        new[i] = pool[i];
    }
    free(pool);
    new[size] = to_append;
    return new;
}

static bool get_diagonal(line_t *line, int x, int y)
{
    int dx = line->finish.x - line->start.x;
    int dy = line->finish.y - line->start.y;
    bool success = false;
    unsigned int counter = 0;

    while (!success) {
        if (counter % 2)
            x = x + (1 * (int)copysign(1, dx));
        else
            y = y + (1 * (int)copysign(1, dy));
        counter++;
        line->size++;
        line->pool = append_point(line->pool, line->size, (point_t) {.x = x, .y = y});
        if (x == line->finish.x && y == line->finish.y) {
            line->size++;
            line->pool = append_point(line->pool, line->size, (point_t) {.x = x, .y = y});
            success = true;
        }
    }
    return true;
}

static dir_name get_position_direction_array(const int current_direction[2])
{
    dir_name direction = -1;

    for (int i = 0; i < 4; i++) {
        if (current_direction[0] == directions[i][0] && current_direction[1] == directions[i][1]) {
            direction = i;
            return direction;
        }
    }
    return direction;
}

static void set_next_pos(room_t *room, point_t *next, const dir_name directions_path[4])
{
    for (int i = 0; i < 4; i++) {
        printf("x=%d\n", next->x);
        printf("y=%d\n", next->y);
        if (room->room[next->y + directions[directions_path[i]][1]][next->x + directions[directions_path[i]][0]] != OBST) {
            next->x = next->x + directions[directions_path[i]][0];
            next->y = next->y + directions[directions_path[i]][1];
            return;
        }
    }
}


//////////////////////////////////
/** NON STATIC FUNCTION BELOW **/
/////////////////////////////////

bool get_line(line_t *line)
{
    int x = line->start.x;
    int y = line->start.y;

    line->size = 0;
    line->pool = malloc(sizeof(point_t));
    line->pool[0] = line->start;
    while (abs(line->dx) != abs(line->dy)) {
        if (abs(line->dx) > abs(line->dy)) {
            x = x + (1 * (int)copysign(1, line->dx));
            line->dx = line->finish.x - x;
            line->size++;
            line->pool = append_point(line->pool, line->size, (point_t) { .x = x, .y = y });
        } else {
            y = y + (1 * (int)copysign(1, line->dy));
            line->dy = line->finish.y - y;
            line->size++;
            line->pool = append_point(line->pool, line->size, (point_t) { .x = x, .y = y });
        }
    }
    if (!get_diagonal(line, x, y)) {
        return false;
    }
    return true;
}

point_t get_first_obst(const line_t* diagonal, const point_t start, const point_t finish, const room_t *room)
{
    int sign_x = (start.x < finish.x) ? 1 : -1;
    int sign_y = (start.y < finish.y) ? 1 : -1;
    unsigned int width = room->width;
    unsigned int height = room->height;

    for (unsigned int i = 0, x, y; i < diagonal->size; i++) {
        x = diagonal->pool[i].x;
        y = diagonal->pool[i].y;
        if (room->room[diagonal->pool[i].y][diagonal->pool[i].x] == OBST) {
            return (point_t) { .x = diagonal->pool[i].x, .y = diagonal->pool[i].y };
        }
        else if (diagonal->pool[i].x > 0 && diagonal->pool[i].y > 0) {
            if (sign_x == 1) {
                if (sign_y == 1) {
                    if (x < width && y < height) {
                        if (room->room[y + 1][x] == OBST && room->room[y][x + 1] == OBST) {
                            return (point_t) { .x = x, .y = y };
                        }
                    }
                }
                else {
                    if (x < width && y > 0) {
                        if (room->room[y - 1][x] == OBST && room->room[y][x + 1] == OBST) {
                            return (point_t) { .x = x, .y = y };
                        }
                    }
                }
            }
            else {
                if (sign_y == 1) {
                    if (x > 0 && y < height) {
                        if (room->room[y + 1][x] == OBST && room->room[y][x - 1] == OBST) {
                            return (point_t) { .x = x, .y = y };
                        }
                    }
                }
                else {
                    if (x > 0 && y > 0) {
                        if (room->room[y - 1][x] == OBST && room->room[y][x - 1] == OBST) {
                            return (point_t) { .x = x, .y = y };
                        }
                    }
                }
            }
        }
    }
    return (point_t) { .x = -1, .y = -1 };
}

bool look_next_position(const room_t *room, const line_t *line, point_t start, point_t *tmp)
{
    dir_name order[4] = {-1};

    if (line->dx > 0) {
        if (line->dy > 0) {
            insert_enum(order, (dir_name[4]) {RIGHT, BOTTOM, LEFT, TOP});
        } else {
            insert_enum(order, (dir_name[4]) {TOP, RIGHT, BOTTOM, LEFT});
        }
    } else { //if dx < 0
        if (line->dy > 0) {
            insert_enum(order, (dir_name[4]) {BOTTOM, LEFT, TOP, RIGHT});
        } else { // if dx < 0 && if dy < 0
            insert_enum(order, (dir_name[4]) {LEFT, TOP, RIGHT, BOTTOM});
        }
    }
    for (int i = 0; i < 4; i++) {
        // we're testing all 4 positions around one given (which is the start)
        if (room->room[start.y + directions[order[i]][1]][start.x + directions[order[i]][0]] != OBST) {
            tmp->x = start.x + directions[order[i]][0];
            tmp->y = start.y + directions[order[i]][1];
            return true;
        }
    }
    return false;
}

void get_order_position(room_t *room, line_t *line, point_t *next)
{
    int current_direction[2] = { // TODO: make define with these conditions (how dirty!)
            ((int)copysign(1, line->dx) == 1 && (int)copysign(1, line->dy) == 1) ? 1 :
            ((int)copysign(1, line->dx) == -1 && (int)copysign(1, line->dy) == -1) ? -1 : 0,
            ((int)copysign(1, line->dx) == -1 && (int)copysign(1, line->dy) == 1) ? 1 :
            ((int)copysign(1, line->dx) == 1 && (int)copysign(1, line->dy) == -1) ? -1 : 0
    };
    dir_name directions_path[4];

    directions_path[0] = get_position_direction_array(current_direction) + 1;
    if (directions_path[0] > 3)
        directions_path[0] = 0;
    for (int i = 1; i < 4; i++) {
        directions_path[i] = directions_path[i - 1] + 1;
        if (directions_path[i] > 3)
            directions_path[i] = 0;
    }
    set_next_pos(room, next, directions_path);
}

/// this is to be used once only, right after get_line()
bool check_fallback(line_t *line, point_t *to_check)
{
    bool result = false;

    printf("test: x=%d\ty=%d\n", to_check->x, to_check->y);
    for (unsigned int i = 0; i < line->size; i++) {
        if (line->pool[i].x == to_check->x && line->pool[i].y == to_check->y) {
            line->size = i + 1;
            to_check->x = line->pool[line->size - 1].x;
            to_check->y = line->pool[line->size - 1].y;
            result = true;
            printf("test: x=%d\ty=%d\n", to_check->x, to_check->y);
            return result;
        }
    }
    printf("test: x=%d\ty=%d\n", to_check->x, to_check->y);
    return result;
}