#include <bits/stdc++.h>
#include <raylib.h>
using namespace std;

const int cell_size = 20;
const int screenwidth = 1200;
const int screenheight = 800;

int rows = screenwidth/cell_size;
int cols = screenheight/cell_size;

vector<vector<int>> grid(rows, vector<int>(cols, false));

int countNeighbours(int x, int y){
    int count = 0;
    for(int dx = -1; dx <= 1; dx++){
        for(int dy = -1; dy <= 1; dy++){
            if(dx == 0 && dy == 0) continue;
            int nx = x+dx, ny = y+dy;
            if(nx >= 0 && nx < rows && ny >= 0 && ny < cols){
                if(grid[nx][ny])
                    count++;
            }
        }
    }
    return count;
}

int countCells(){
    int live = 0;
    for(int x = 0; x < rows; x++){
        for(int y = 0; y < cols; y++){
            if(grid[x][y]) live++;
        }
    }
    return live;
}

void updateGrid(){
    auto next = grid;
    for(int x = 0; x < rows; x++){
        for(int y = 0; y < cols; y++){
            int n = countNeighbours(x, y);
            if(grid[x][y]){
                next[x][y] = (n==2 || n==3);
            }
            else{
                next[x][y] = (n==3);
            }
        }
    }
    grid = next;
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

    InitWindow(screenwidth, screenheight, "conway's game of life");

    while(!WindowShouldClose()){
        BeginDrawing();

        ClearBackground(bg);

        for(int i = cell_size; i < screenwidth; i+= cell_size){
            DrawLine(i, 0, i, screenheight, lines);
        }
        for(int i = cell_size; i < screenheight; i+= cell_size){
            DrawLine(0, i, screenwidth, i, lines);
        }
        for(int x = 0; x < rows; x++){
            for(int y = 0; y < cols; y++){
                if(grid[x][y]){
                    DrawRectangle(x*cell_size, y*cell_size, cell_size, cell_size, cellc);
                }
            }
        }

        DrawText(TextFormat("generation: %d", gen), 10, 10, 20, textc);
        DrawText(TextFormat("live cells: %d", live), 10, 30, 20, textc);
        DrawText(TextFormat("speed: %f", interval), 10, 50, 20, textc);

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            Vector2 mouse = GetMousePosition();
            int row = mouse.x/cell_size;
            int col = mouse.y/cell_size;
            if(row >= 0 && row < rows && col >= 0 && col < cols){
                grid[row][col] = !grid[row][col];
            }
        }

        wheel = GetMouseWheelMove();
        live = countCells();

        if(IsKeyPressed(KEY_SPACE)){
            running = !running;
        }

        if(IsKeyPressed(KEY_C)){
            grid = vector<vector<int>>(rows, vector<int>(cols, false));
            gen = 0;
            live = 0;
        }

        if(IsKeyPressed(KEY_R)){
            gen = 0;
            for(auto &row : grid){
                for(auto &cell : row){
                    cell = (GetRandomValue(0, 100) < 20);
                }
            }
        }

        if(IsKeyPressed(KEY_N)){
            gen++;
            updateGrid();
        }

        if(wheel){
            interval -= wheel*0.01f;
            if(interval < 0.01f) interval = 0.01f;
            else if(interval > 1.0f) interval = 1.0f;
        }

        if(running){
            timer += GetFrameTime();
            if(timer >= interval){
                updateGrid();
                gen++;
                timer = 0.0f;
            }
        }

        EndDrawing();
    }
    CloseWindow();
}