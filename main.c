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

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyDown(KEY_D))
            player.pos.data[0] += 1;
        if (IsKeyDown(KEY_A))
            player.pos.data[0] -= 1;
        if (IsKeyDown(KEY_S))
            player.pos.data[1] += 1;
        if (IsKeyDown(KEY_W))
            player.pos.data[1] -= 1;
        

        Vector2 mpos = GetMousePosition();

        DrawRectangle(other.pos.data[0], other.pos.data[1], other.ext.data[0], other.ext.data[1], WHITE);
        DrawRectangle(player.pos.data[0], player.pos.data[1], player.ext.data[0], player.ext.data[1], WHITE);
        bumpc_Vec goal = bumpc_VecNew(mpos.x, mpos.y);

        {
            bumpc_CollisionResult col = bumpc_detectCollisionAabb(player, other, bumpc_VecNew(mpos.x, mpos.y));
            if(col.status == BUMPC_GOOD){

                DrawText("col", 0, 0, 40, ORANGE);
                if(col.overlaps){
                    DrawText("overlap", 190, 200, 40, ORANGE);
                }

                float sx = player.pos.data[0];
                float sy = player.pos.data[1];
                DrawLine(sx, sy, col.delta.data[0] + sx, col.delta.data[1] + sy, RED);
            }
        }

        EndDrawing();
  }
  CloseWindow();
}
