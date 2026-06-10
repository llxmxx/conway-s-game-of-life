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

enum tool{
    brush, erase, rect, line
};

enum panels{
    def, tools, patterns
};

const int cell_size = 20;
const int screenw = 1200;
const int screenh = 800;

int rows = 2000; //screenw/cell_size;
int cols = 2000; //screenh/cell_size;

Color bg = {34, 40, 49, 255};
Color lines = {57, 62, 70, 255};
Color cellc = {223, 208, 184, 255};
Color textc = {148, 137, 121, 255};

unordered_set<cell, cell_hash> livecells;

Camera2D camera = {0};

void countNeighbours(cell c, unordered_map<cell, int, cell_hash> &neighbours){
    int x = c.x, y = c.y;
    for(int dx = -1; dx <= 1; dx++){
        for(int dy = -1; dy <= 1; dy++){
            if(!dx && !dy) continue;
            int nx = x+dx, ny = y+dy;
            cell nc; nc.x = nx; nc.y = ny;
            neighbours[nc]++;
        }
    }
}

bool buttonClick(Rectangle rect){
    Vector2 mouse = GetMousePosition();
    bool clicked = mouse.x >= rect.x && mouse.x <= rect.x+rect.width && mouse.y >= rect.y && mouse.y <= rect.y+rect.height && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    return clicked;
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
    livecells = nextlive;
}

cell getCell(){
    Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
    int row = mouse.x/cell_size;
    int col = mouse.y/cell_size;
    return {row, col};
}

int main(){
    bool running = false;
    bool isPanelOpen = false;
    bool dragging = false;
    tool currTool = brush;
    panels currPanel = def;
    float timer = 0.0f;
    float interval = 0.1f;
    float wheel = 0;

    int gen = 0;
    int live = 0;

    camera.offset = {screenw/2.0f, screenh/2.0f};
    camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
    camera.zoom = 1.0f;

    Rectangle panel = {screenw-100, 0, 100, screenh};
    Rectangle panelBtn = {screenw-10, screenh-100, 10, 20};
    Rectangle btn1 = {screenw-85, 40, 70, 40};
    Rectangle btn2 = {screenw-85, 120, 70, 40};
    Rectangle btn3 = {screenw-85, 200, 70, 40};
    Rectangle btn4 = {screenw-85, 280, 70, 40};
    Rectangle btn5 = {screenw-85, 360, 70, 40};
    Rectangle btn6 = {screenw-85, 420, 70, 40};
    Rectangle btn7 = {screenw-85, 500, 70, 40};
    Rectangle btn8 = {screenw-85, 580, 70, 40};

    cell startc, endc;

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

        if(isPanelOpen){
            DrawRectangleRec(panel, lines);
            DrawRectangleRec(panelBtn, textc);
            DrawText(">", panelBtn.x+1, panelBtn.y, 20, bg);
            switch(currPanel){
                case def:
                DrawRectangleRec(btn1, textc);
                DrawText("tools", btn1.x+8, btn1.y+8, 20, bg);
                DrawRectangleRec(btn2, textc);
                DrawText("patterns", btn2.x+1, btn2.y+8, 15, bg);
                break;
                case tools:
                DrawRectangleRec(btn1, textc);
                DrawText("brush", btn1.x+1, btn1.y+8, 20, bg);
                DrawRectangleRec(btn2, textc);
                DrawText("erase", btn2.x+1, btn2.y+8, 20, bg);
                DrawRectangleRec(btn3, textc);
                DrawText("rect", btn3.x+8, btn3.y+8, 20, bg);
                DrawRectangleRec(btn4, textc);
                DrawText("line", btn4.x+12, btn4.y+8, 20, bg);
                DrawRectangleRec(btn8, textc);
                DrawText("back", btn8.x+8, btn8.y+8, 20, bg);
                break;
                case patterns:
                DrawRectangleRec(btn8, textc);
                DrawText("back", btn8.x+8, btn8.y+8, 20, bg);
            }
        }
        else{
            DrawRectangleRec(panelBtn, textc);
            DrawText("<", panelBtn.x+1, panelBtn.y, 20, bg);
        }
        DrawText(TextFormat("generations: %d", gen), 10, 10, 20, textc);
        DrawText(TextFormat("live cells: %d", live), 10, 30, 20, textc);
        DrawText(TextFormat("speed: %.2fx", 2-interval), 10, 50, 20, textc);

        EndDrawing();

        wheel = GetMouseWheelMove();
        live = livecells.size();

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            bool used = false;
            if(buttonClick(panelBtn)){
                isPanelOpen = !isPanelOpen;
                used = true;
            }
            else if(isPanelOpen){
                switch(currPanel){
                    case def:
                    if(buttonClick(btn1)){
                        currPanel = tools;
                        used = true;
                    }
                    if(buttonClick(btn2)){
                        currPanel = patterns;
                        used = true;
                    }
                    break;
                    case tools:
                    if(buttonClick(btn1)){
                        currTool = brush;
                        used = true;
                    }
                    else if(buttonClick(btn2)){
                        currTool = erase;
                        used = true;
                    }
                    else if(buttonClick(btn3)){
                        currTool = rect;
                        used = true;
                    }
                    else if(buttonClick(btn4)){
                        currTool = line;
                        used = true;
                    }
                    else if(buttonClick(btn8)){
                        currPanel = def;
                        used = true;
                    }
                    break;
                    case patterns:
                    if(buttonClick(btn8)){
                        currPanel = def;
                        used = true;
                    }
                }
                
            }
            if(!used && (!isPanelOpen || !buttonClick(panel))){
                dragging = true;
                if(currTool == rect || currTool == line){
                    startc = getCell();
                }
            }
        }

        if(dragging){
            gen = 0;
            cell c = getCell();
            switch(currTool){
                case brush:
                livecells.emplace(c);
                break;
                case erase:
                livecells.erase(c);
                break;
            }
        }

        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragging){
            endc = getCell();
            switch(currTool){
                case rect:
                for(int x = min(startc.x, endc.x); x <= max(startc.x, endc.x); x++){
                    for(int y = min(startc.y, endc.y); y <= max(startc.y, endc.y); y++){
                        cell c = {x, y};
                        livecells.emplace(c);
                    }
                }
                break;
                case line:
                float dx = startc.x - endc.x, dy = startc.y - endc.y;
                int steps = max(abs(dx), abs(dy));
                if (steps == 0) break;
                dx /= steps;
                dy /= steps;
                for(int i = 0; i < abs(steps); i++){
                    float x = startc.x-dx*i, y = startc.y-dy*i;
                    cell c = {(int)round(x), (int)round(y)};
                    livecells.emplace(c);
                }
                break;
            }
            dragging = false;
        }

        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)){
            if(!isPanelOpen || !buttonClick(panel)){
                Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), camera);
                int row = mouse.x/cell_size;
                int col = mouse.y/cell_size;
                if(row >= 0 && row < rows && col >= 0 && col < cols){
                    cell c = {row, col};
                    if(currTool == erase) livecells.emplace(c);
                    else livecells.erase(c);
                }
                gen = 0;
            }
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
            float scale = 0.05f*wheel;
            float zoomval = expf(logf(camera.zoom)+scale);
            if(zoomval > 20.0f) zoomval = 20.0f;
            else if(zoomval < 0.5f) zoomval = 0.5f;
            camera.zoom = zoomval;
        }

        if(IsKeyPressed(KEY_SPACE)){
            running = !running;
        }

        if(IsKeyPressed(KEY_C)){
            livecells.clear();
            gen = 0;
            live = 0;
            camera.zoom = 1.0f;
            camera.offset = {screenw/2.0f, screenh/2.0f};
            camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
        }

        if(IsKeyPressed(KEY_R)){
            gen = 0;
            livecells.clear();
            for(int i = startx; i < endx; i++){
                for(int j = starty; j < endy; j++){
                    if(GetRandomValue(0, 100) < 20){
                        livecells.emplace((cell){i, j});
                    }
                }
            }
        }

        if(IsKeyPressed(KEY_N)){
            gen++;
            updateGrid();
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
                timer = 0.0f;
            }
        }
    }

    CloseWindow();
}