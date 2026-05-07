/*
 * camera.h — Smooth-follow camera system
 */
#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

/* Initialize camera centered on target */
void camera_init(Camera2D *camera, Vec2 target);

/* Smoothly follow a target position */
void camera_update(Camera2D *camera, Vec2 target, float dt);

/* Apply screen shake effect */
void camera_shake(Game *game, float intensity, float duration);

/* Update screen shake */
void camera_update_shake(Game *game, float dt);

#endif /* CAMERA_H */
