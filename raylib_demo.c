#include <stdio.h>
#include "raylib.h"
#include "bump.h"
#include <stdlib.h>

void main(){
    int scrw = 800;
    int scrh = 600;
    printf("Hello\n");
    InitWindow(scrw,scrh, "get spooky");
    SetTargetFPS(60);

    bumpc_Ctx bctx = bumpc_init();

    size_t rect_ct = 50;
    bumpc_Aabb * rects  = malloc(sizeof(bumpc_Aabb) * rect_ct);
    for(int i = 0; i < rect_ct; i++){
        rects[i].pos = bumpc_VecNew(rand() % scrw, rand() % scrh);
        rects[i].ext = bumpc_VecNew(rand() % 100 + 20, rand() % 100 + 20);
    }

    //aligned rects
    rects[0].pos = bumpc_VecNew(40,40);
    rects[0].ext = bumpc_VecNew(40,40);
    rects[1].ext = bumpc_VecNew(40,40);
    rects[1].pos = bumpc_VecNew(40 * 2,40);

    int bw = 10;
    rects[2].pos = bumpc_VecNew(-bw,0);
    rects[2].ext = bumpc_VecNew(bw, scrh);

    rects[3].pos = bumpc_VecNew(0,-bw);
    rects[3].ext = bumpc_VecNew(scrw, bw);

    rects[4].pos = bumpc_VecNew(0,scrh);
    rects[4].ext = bumpc_VecNew(scrw, bw);

    rects[5].pos = bumpc_VecNew(scrw,0);
    rects[5].ext = bumpc_VecNew(bw, scrh);

    bumpc_Aabb player = {0};
    player.ext.data[0] =  30;
    player.ext.data[1] =  30;


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if(player.pos.data[0] < -bw || player.pos.data[1] < -bw  || player.pos.data[0] > scrw || player.pos.data[1] > scrh){
            player.pos = bumpc_VecNew(0,0);
        }

        float dx = 0;
        float dy = 0;
        bool dorand = true;

        float mul = 100;
        if(dorand){
            dx += rand() % 3 - 1;
            dy += rand() % 3 - 1;
        }
        else{
        if (IsKeyDown(KEY_D))
            dx += 1;
        if (IsKeyDown(KEY_A))
            dx -= 1;
        if (IsKeyDown(KEY_S))
            dy += 1;
        if (IsKeyDown(KEY_W))
            dy -= 1;
        }

        if(IsKeyDown(KEY_SPACE))
            player.pos.data[0] += 10;
        
        dx *= mul;
        dy *= mul;

        for(int i = 0; i < rect_ct; i++){
            const bumpc_Aabb re = rects[i];
            DrawRectangleLines(re.pos.data[0], re.pos.data[1], re.ext.data[0], re.ext.data[1], WHITE);
        }


        DrawRectangle(player.pos.data[0], player.pos.data[1], player.ext.data[0], player.ext.data[1], WHITE);
        bumpc_Vec goal = bumpc_VecNew(player.pos.data[0] + dx, player.pos.data[1] + dy);
        bumpc_Aabb moved = player;

        size_t counter = 0;
        while(counter < 100){
            counter += 1;
            size_t num_col = bumpc_detectCollisionList(&bctx, moved, rects, rect_ct, goal);
            //TODO prevent infinite loop
            if(num_col == 0)
                break;
            for(int i = 0; i < num_col; i++){
                bumpc_CollisionResult r  = bctx.output[i];
                if(r.overlaps)
                    printf("overlapped\n");

                moved.pos = r.touch;
                if(r.norm.data[1] != 0){
                    goal.data[1]  = r.touch.data[1];
                    break;
                }
                else {
                    goal.data[0]  = r.touch.data[0];
                    break;
                }
            }
        }
        if(counter > 90)
            printf("maxed\n");
        player.pos = goal;

        EndDrawing();
  }
  CloseWindow();
}
