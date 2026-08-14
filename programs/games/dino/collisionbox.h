#ifndef COLLISIONBOX_H
#define COLLISIONBOX_H

// All box coordinates fit in a byte (max value is 58)
typedef struct {
    signed char x;
    signed char y;
    signed char width;
    signed char height;
} CollisionBox;

#endif
