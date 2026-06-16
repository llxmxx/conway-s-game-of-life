#include <bits/stdc++.h>
#include <raylib.h>
using namespace std;
#define endl '\n'

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
    def, tools, patterns, stats
};

enum pattern{
    nor, glider, lwss, mwss, hwss, gosper, pulsar, pdthlon, acorn, rpento
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

map<pattern, vector<cell>> pat = {
    {glider, {{0, -1}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}},
    {lwss, {{-2, -2}, {1, -2}, {2, -1}, {-2, 0}, {2, 0}, {-1, 1}, {0, 1}, {1, 1}, {2, 1}}},
    {mwss, {{-2, -2}, {-1, -2}, {0, -2}, {-3, -1}, {-2, -1}, {-1, -1}, {0, -1}, {1, -1}, {-3, 0}, {-2, 0}, {-1, 0}, {1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    {hwss, {{0, -1}, {1, -1}, {-4, 0}, {-3, 0}, {-2, 0}, {-1, 0}, {1, 0}, {2, 0}, {-4, 1}, {-3, 1}, {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {-3, 2}, {-2, 2}, {-1, 2}, {0, 2}}},
    {gosper, {{6, -4}, {4, -3}, {6, -3}, {-6, -2}, {-5, -2}, {2, -2}, {3, -2}, {16, -2}, {17, -2}, {-7, -1}, {-3, -1}, {2, -1}, {3, -1}, {16, -1}, {17, -1}, {-18, 0}, {-17, 0}, {-8, 0}, {-2, 0}, {2, 0}, {3, 0}, {-18, 1}, {-17, 1}, {-8, 1}, {-4, 1}, {-2, 1}, {-1, 1}, {4, 1}, {6, 1}, {-8, 2}, {-2, 2}, {6, 2}, {-7, 3}, {-3, 3}, {-6, 4}, {-5, 4}}},
    {pulsar, {{-4, -6}, {-3, -6}, {-2, -6}, {2, -6}, {3, -6}, {4, -6}, {-6, -4}, {-1, -4}, {1, -4}, {6, -4}, {-6, -3}, {-1, -3}, {1, -3}, {6, -3}, {-6, -2}, {-1, -2}, {1, -2}, {6, -2}, {-4, -1}, {-3, -1}, {-2, -1}, {2, -1}, {3, -1}, {4, -1}, {-4, 1}, {-3, 1}, {-2, 1}, {2, 1}, {3, 1}, {4, 1}, {-6, 2}, {-1, 2}, {1, 2}, {6, 2}, {-6, 3}, {-1, 3}, {1, 3}, {6, 3}, {-6, 4}, {-1, 4}, {1, 4}, {6, 4}, {-4, 6}, {-3, 6}, {-2, 6}, {2, 6}, {3, 6}, {4, 6}}},
    {pdthlon, {{-2, -1}, {3, -1}, {-4, 0}, {-3, 0}, {-1, 0}, {0, 0}, {1, 0}, {2, 0}, {4, 0}, {5, 0}, {-2, 1}, {3, 1}}},
    {acorn, {{-2, -2}, {0, -1}, {-3, 0}, {-2, 0}, {1, 0}, {2, 0}, {3, 0}}},
    {rpento, {{0, -1}, {1, -1}, {-1, 0}, {0, 0}, {0, 1}}}
};

int gen;
int live;
int peak;
int total;
int minx, maxx;
int miny, maxy;

vector<int> populationGraph;

unordered_set<cell, cell_hash> livecells;

Camera2D camera = {0};

void checkBounds(){
    for(cell c : livecells){
        int x = c.x, y = c.y;
        if (x < minx) minx = x;
        else if(x > maxx) maxx = x;
        if(y < miny) miny = y;
        else if(y > maxy) maxy = y;
    }
}

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

void reset(){
    livecells.clear();
    gen = 0;
    live = 0;
    peak = 0;
    minx = 4e6;
    maxx = -4e6;
    miny = 4e6;
    maxx = -4e6;
    total = 0;
    populationGraph.clear();
}

int main(){
    reset();

    bool running = false;
    bool isPanelOpen = false;
    bool dragging = false;
    tool currTool = brush;
    panels currPanel = def;
    pattern currPattern = nor;
    float timer = 0.0f;
    float interval = 0.1f;
    float wheel = 0;

    camera.offset = {screenw/2.0f, screenh/2.0f};
    camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
    camera.zoom = 1.0f;

    float btnx = screenw-85;
    float btnw = 70, btnh = 30;

    Rectangle panel = {screenw-200, 0, 200, screenh};
    Rectangle panelBtn = {screenw-10, screenh-100, 10, 20};
    Rectangle btn1 = {btnx, 30, btnw, btnh};
    Rectangle btn2 = {btnx, 100, btnw, btnh};
    Rectangle btn3 = {btnx, 170, btnw, btnh};
    Rectangle btn4 = {btnx, 240, btnw, btnh};
    Rectangle btn5 = {btnx, 310, btnw, btnh};
    Rectangle btn6 = {btnx, 380, btnw, btnh};
    Rectangle btn7 = {btnx, 450, btnw, btnh};
    Rectangle btn8 = {btnx, 520, btnw, btnh};
    Rectangle btn9 = {btnx, 590, btnw, btnh};
    Rectangle btn10 = {btnx, 660, btnw, btnh};
    Rectangle btn11 = {btnx, 730, btnw, btnh};

    int graphx = panel.x+5, graphy = panel.y+180;
    int graphw = 190, graphh = 380;

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

        checkBounds();
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
                DrawRectangleRec(btn3, textc);
                DrawText("stats", btn3.x+8, btn3.y+8, 20, bg);
                DrawRectangleRec(btn4, textc);
                DrawText("save", btn4.x+8, btn4.y+8, 20, bg);
                DrawRectangleRec(btn5, textc);
                DrawText("load", btn5.x+8, btn5.y+8, 20, bg);
                DrawRectangleRec(btn6, textc);
                DrawText("import", btn6.x+5, btn6.y+8, 20, bg);
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
                DrawRectangleRec(btn11, textc);
                DrawText("back", btn11.x+8, btn11.y+8, 20, bg);
                break;
                case patterns:
                DrawRectangleRec(btn1, textc);
                DrawText("normal", btn1.x, btn1.y+8, 20, bg);
                DrawRectangleRec(btn2, textc);
                DrawText("glider", btn2.x, btn2.y+8, 20, bg);
                DrawRectangleRec(btn3, textc);
                DrawText("lwss", btn3.x+1, btn3.y+8, 20, bg);
                DrawRectangleRec(btn4, textc);
                DrawText("mwss", btn4.x+1, btn4.y+8, 20, bg);
                DrawRectangleRec(btn5, textc);
                DrawText("hwss", btn5.x+1, btn5.y+8, 20, bg);
                DrawRectangleRec(btn6, textc);
                DrawText("gosper", btn6.x, btn6.y+8, 20, bg);
                DrawRectangleRec(btn7, textc);
                DrawText("pulsar", btn7.x, btn7.y+8, 20, bg);
                DrawRectangleRec(btn8, textc);
                DrawText("pdthlon", btn8.x, btn8.y+8, 20, bg);
                DrawRectangleRec(btn9, textc);
                DrawText("acorn", btn9.x, btn9.y+8, 20, bg);
                DrawRectangleRec(btn10, textc);
                DrawText("rpento", btn10.x, btn10.y+8, 20, bg);
                DrawRectangleRec(btn11, textc);
                DrawText("back", btn11.x+8, btn11.y+8, 20, bg);
                break;
                case stats:
                DrawText(TextFormat("peak\npopulation: %d", peak), panel.x+5, panel.y+20, 20, textc);
                if (livecells.empty()) DrawText("bounding box:\n0 x 0", panel.x+5, panel.y+70, 20, textc);
                else DrawText(TextFormat("bounding box:\n%d x %d", maxx-minx+1, maxy-miny+1), panel.x+5, panel.y+70, 20, textc);
                if(gen) DrawText(TextFormat("avg population: \n%d", total/gen), panel.x+5, panel.y+120, 20, textc);
                else DrawText("avg population: 0", panel.x+5, panel.y+120, 20, textc);
                if(populationGraph.size() > 1){
                    int maxPop = 1;
                    for(int p : populationGraph){
                        maxPop = max(maxPop, p);
                    }
                    for(int i = 1; i < populationGraph.size(); i++){
                        float x1 = graphx + (float)(i-1) * graphw/(populationGraph.size()-1);
                        float y1 = graphy + graphh - (float)populationGraph[i-1]*graphh/maxPop;
                        float x2 = graphx + (float)i * graphw/(populationGraph.size()-1);
                        float y2 = graphy + graphh - (float)populationGraph[i]*graphh/maxPop;
                        DrawLine(x1, y1, x2, y2, textc);
                    }
                }
                DrawRectangleRec(btn11, textc);
                DrawText("back", btn11.x+8, btn11.y+8, 20, bg);
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
        peak = (live > peak ? live : peak);

        if(populationGraph.size() > 500){
            populationGraph.erase(populationGraph.begin());
        }

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
                    if(buttonClick(btn3)){
                        currPanel = stats;
                        used = true;
                    }
                    if(buttonClick(btn4) && !running){
                        ofstream file("saves/save.life");
                        file << gen << endl;
                        file << camera.offset.x << ' ' << camera.offset.y << endl;
                        file << camera.target.x << ' ' << camera.target.y << endl;
                        file << camera.zoom << endl;
                        file << live << endl;
                        for(auto c : livecells){
                            file << c.x << ' ' << c.y << endl;
                        }
                        file.close();
                    }
                    if(buttonClick(btn5) && !running){
                        ifstream file("saves/save.life");
                        livecells.clear();
                        file >> gen >> camera.offset.x >> camera.offset.y >> camera.target.x >> camera.target.y >> camera.zoom >> live;
                        for(int i = 0; i < live; i++){
                            cell c;
                            file >> c.x >> c.y;
                            livecells.emplace(c);
                        }
                        file.close();
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
                    else if(buttonClick(btn11)){
                        currPanel = def;
                        used = true;
                    }
                    break;
                    case patterns:
                    if(buttonClick(btn1)){
                        currPattern = nor;
                        used = true;
                    }
                    else if(buttonClick(btn2)){
                        currPattern = glider;
                        used = true;
                    }
                    else if(buttonClick(btn3)){
                        currPattern = lwss;
                        used = true;
                    }
                    else if(buttonClick(btn4)){
                        currPattern = mwss;
                        used = true;
                    }
                    else if(buttonClick(btn5)){
                        currPattern = hwss;
                        used = true;
                    }
                    else if(buttonClick(btn6)){
                        currPattern = gosper;
                        used = true;
                    }
                    else if(buttonClick(btn7)){
                        currPattern = pulsar;
                        used = true;
                    }
                    else if(buttonClick(btn8)){
                        currPattern = pdthlon;
                        used = true;
                    }
                    else if(buttonClick(btn9)){
                        currPattern = acorn;
                        used = true;
                    }
                    else if(buttonClick(btn10)){
                        currPattern = rpento;
                        used = true;
                    }
                    else if(buttonClick(btn11)){
                        currPanel = def;
                        used = true;
                    }
                    break;
                    case stats:
                    if(buttonClick(btn11)){
                        currPanel = def;
                        used = true;
                    }
                }
            }
            if(!isPanelOpen || !buttonClick(panel)){
                dragging = true;
                if(currTool == rect || currTool == line){
                    startc = getCell();
                }
                else if(currPattern != nor){
                    dragging = false;
                    cell c = getCell();
                    int n = pat[currPattern].size();
                    for(int i = 0; i < n; i++){
                        cell nc = {c.x+pat[currPattern][i].x, c.y+pat[currPattern][i].y};
                        livecells.emplace(nc);
                    }
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
            reset();
            camera.zoom = 1.0f;
            camera.offset = {screenw/2.0f, screenh/2.0f};
            camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
        }

        if(IsKeyPressed(KEY_R)){
            reset();
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
            total += live;
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
                if(interval <= 0.1f)
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
                populationGraph.push_back(live);
                total += live;
                timer = 0.0f;
            }
        }
    }

    CloseWindow();
}