#include <stdio.h>
#include "raylib.h"
#include "bump.c"
#include <stdlib.h>

void main(){
    printf("Hello\n");
    InitWindow(800,600, "get spooky");
    SetTargetFPS(60);

    size_t rect_ct = 30;
    bumpc_Aabb * rects  = malloc(sizeof(bumpc_Aabb) * rect_ct);
    for(int i = 0; i < rect_ct; i++){
        rects[i].pos = bumpc_VecNew(rand() % 800, rand() % 600);
        rects[i].ext = bumpc_VecNew(rand() % 100 + 20, rand() % 100 + 20);
    }

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

        for(int i = 0; i < rect_ct; i++){
            const bumpc_Aabb re = rects[i];
            DrawRectangle(re.pos.data[0], re.pos.data[1], re.ext.data[0], re.ext.data[1], WHITE);
        }


        DrawRectangle(player.pos.data[0], player.pos.data[1], player.ext.data[0], player.ext.data[1], WHITE);
        bumpc_Vec goal = bumpc_VecNew(player.pos.data[0] + dx, player.pos.data[1] + dy);

        size_t num_col = bumpc_detectCollisionList(player, rects, rect_ct, goal, cols, &cols_len);
        for(int i = 0; i < num_col; i++){
            
            bumpc_CollisionResult r  = cols[i];
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
