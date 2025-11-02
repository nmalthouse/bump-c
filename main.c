#include <stdio.h>
#include "raylib.h"
#include "bump.c"

void main(){
    printf("Hello\n");
    InitWindow(800,600, "get spooky");
    SetTargetFPS(60);

    bumpc_Aabb other;
    other.pos = bumpc_VecNew(100,200);
    other.ext = bumpc_VecNew(100,100);

    bumpc_Aabb player = {0};
    player.ext.data[0] =  30;
    player.ext.data[1] =  30;

    size_t cols_len = 5;
    bumpc_CollisionResult * cols = malloc(sizeof(bumpc_CollisionResult) * cols_len);


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        float dx = 0;
        float dy = 0;

        if (IsKeyDown(KEY_D))
            dx += 1;
        if (IsKeyDown(KEY_A))
            dx -= 1;
        if (IsKeyDown(KEY_S))
            dy += 1;
        if (IsKeyDown(KEY_W))
            dy -= 1;
        
        dx *= 1.5;
        dy *= 1.5;


        DrawRectangle(other.pos.data[0], other.pos.data[1], other.ext.data[0], other.ext.data[1], WHITE);
        DrawRectangle(player.pos.data[0], player.pos.data[1], player.ext.data[0], player.ext.data[1], WHITE);
        bumpc_Vec goal = bumpc_VecNew(player.pos.data[0] + dx, player.pos.data[1] + dy);

        size_t num_col = bumpc_detectCollisionList(player, &other, 1, goal, cols, &cols_len);
        if(num_col > 0){
            bumpc_CollisionResult r  = cols[0];
            bumpc_Vec new = player.pos;
            if(r.norm.data[0] != 0){
                goal.data[0]  = r.touch.data[0];
            }
            if(r.norm.data[1] != 0)
                goal.data[1]  = r.touch.data[1];
                new.data[1] = r.touch.data[1];
        }
            player.pos = goal;

        EndDrawing();
  }
  CloseWindow();
}
