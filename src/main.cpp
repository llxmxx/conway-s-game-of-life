#include <bits/stdc++.h>
#include <raylib.h>
using namespace std;

struct cell{
    int x;
    int y;

    bool operator == (const cell& other) const{
        return x==other.x && y==other.y;
    }
};

struct cell_hash{
    size_t operator()(const cell& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
    }
};

const int cell_size = 20;
const int screenw = 1200;
const int screenh = 800;

int rows = 2000; //screenw/cell_size;
int cols = 2000; //screenh/cell_size;

unordered_set<cell, cell_hash> livecells;

void countNeighbours(cell c, unordered_map<cell, int, cell_hash> &neighbours){
    int x = c.x, y = c.y;
    for(int dx = -1; dx <= 1; dx++){
        for(int dy = -1; dy <= 1; dy++){
            if(!dx && !dy) continue;
            int nx = x+dx, ny = y+dy;
            if(nx >= 0 && nx < rows && ny >= 0 && ny < cols){
                cell nc; nc.x = nx; nc.y = ny;
                neighbours[nc]++;
            }
        }
    }
}

void updateGrid(){
    unordered_map<cell, int, cell_hash> neighbours;
    unordered_set<cell, cell_hash> nextlive;
    for(cell c : livecells){
        countNeighbours(c, neighbours);
    }
    for(auto &[c, n] : neighbours){
        if(n==2 && livecells.find(c) != livecells.end()){
            nextlive.emplace(c);
        }
        else if(n==3){
            nextlive.emplace(c);
        }
    }
}

int countCells(){
    int live = livecells.size();
    return live;
}

int main(){
    bool running = false;
    float timer = 0.0f;
    float interval = 0.1f;
    float wheel = 0;

    int gen = 0;
    int live = 0;

    Color bg = {34, 40, 49, 255};
    Color lines = {57, 62, 70, 255};
    Color cellc = {223, 208, 184, 255};
    Color textc = {148, 137, 121, 255};

    Camera2D camera = {0};
    camera.offset = {screenw/2.0f, screenh/2.0f};
    camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    InitWindow(screenw, screenh, "conway's game of life");

    while(!WindowShouldClose()){

        Vector2 topleft = GetScreenToWorld2D({0, 0}, camera);
        Vector2 bottomright = GetScreenToWorld2D({(float)screenw, (float)screenh}, camera);

        int startx = max(0, (int)(topleft.x/cell_size)-1);
        int endx = min(rows, (int)(bottomright.x/cell_size)+1);
        int starty = max(0, (int)(topleft.y/cell_size)-1);
        int endy = min(cols, (int)(bottomright.y/cell_size)+1);

        BeginDrawing();

        ClearBackground(bg);

        BeginMode2D(camera);

        for(int i = startx*cell_size; i < endx*cell_size; i += cell_size){
            DrawLine(i, starty, i, endy*cell_size, lines);
        }
        for(int i = starty*cell_size; i < endy*cell_size; i += cell_size){
            DrawLine(startx, i, endx*cell_size, i, lines);
        }
        for(int x = startx; x < endx; x++){
            for(int y = starty; y < endy; y++){
                if(livecells.find({x, y}) != livecells.end()){
                    DrawRectangle(x*cell_size, y*cell_size, cell_size, cell_size, cellc);
                }
            }
        }

        EndMode2D();

        DrawText(TextFormat("generations: %d", gen), 10, 10, 20, textc);
        DrawText(TextFormat("live cells: %d", live), 10, 30, 20, textc);
        DrawText(TextFormat("speed: %.2fx", 2-interval), 10, 50, 20, textc);

        EndDrawing();

        wheel = GetMouseWheelMove();

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
            int row = mouse.x/cell_size;
            int col = mouse.y/cell_size;
            if(row >= 0 && row < rows && col >= 0 && col < cols){
                cell c = {row, col};
                if(livecells.find(c) != livecells.end()){
                    livecells.erase(c);
                }
                else{
                    livecells.emplace(c);
                }
            }
            gen = 0;
            live = countCells();
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            Vector2 delta = GetMouseDelta();
            delta = {delta.x*(-1.0f/camera.zoom), delta.y*(-1.0f/camera.zoom)};
            camera.target.x += delta.x;
            camera.target.y += delta.y;
        }

        if(wheel){
            Vector2 mouseworldpos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseworldpos;
            float scale = 0.09f*wheel;
            float zoomval = expf(logf(camera.zoom)+scale);
            if(zoomval > 40.0f) zoomval = 40.0f;
            else if(zoomval < 0.125f) zoomval = 0.125f;
            camera.zoom = zoomval;
        }

        if(IsKeyPressed(KEY_SPACE)){
            running = !running;
        }

        if(IsKeyPressed(KEY_C)){
            livecells.clear();
            gen = 0;
            live = 0;
        }

        if(IsKeyPressed(KEY_R)){
            gen = 0;
            for(int i = startx; i < endx; i++){
                for(int j = starty; j < endy; j++){
                    if(GetRandomValue(0, 100) < 20){
                        livecells.emplace((cell){i, j});
                    }
                }
            }
            live = countCells();
        }

        if(IsKeyPressed(KEY_N)){
            gen++;
            updateGrid();
            live = countCells();
        }

        if(IsKeyPressed(KEY_MINUS)){
            if(interval < 1.0f){
                if(interval < 0.1f)
                    interval += 0.01f;
                else
                    interval += 0.1f;
            }
        }

        if(IsKeyPressed(KEY_EQUAL)){
            if(interval > 0.01f){
                if(interval < 0.1f)
                    interval -= 0.01f;
                else
                    interval -= 0.1f;
            }
        }

        if(running){
            timer += GetFrameTime();
            if(timer >= interval){
                updateGrid();
                gen++;
                live = countCells();
                timer = 0.0f;
            }
        }
    }

    CloseWindow();
}