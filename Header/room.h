//
// Created by valentin on 21/09/2019.
//

#ifndef MAPGENERATION_ROOM_H
#define MAPGENERATION_ROOM_H

#include "core_game.h"

room_t initialize_room(const int width, const int height);
void define_hash(const int size_map);
float perlin2d(float x, float y, float freq, int depth);
void free_hash(void);

#endif //MAPGENERATION_ROOM_H
