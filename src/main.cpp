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
    brush, erase, rect, line, sel
};

enum panels{
    def, tools, patterns, stats, load
};

enum pattern{
    nor, glider, lwss, mwss, hwss, gosper, pulsar, pdthlon, acorn, rpento, cop, selpat
};

struct state{
    unordered_set<cell, cell_hash> livecells;
    tool currTool = brush;
    panels currPanel = def;
    pattern currPattern = nor;
    Camera2D camera = {0};
};

state start;

const int cell_size = 20;
const int screenw = 1200;
const int screenh = 800;

int rows = 2000; //screenw/cell_size;
int cols = 2000; //screenh/cell_size;

Color bg = {15, 23, 42, 255};
Color lines = {37, 47, 53, 255};
Color btnc = {51, 65, 85, 255};
Color cellc = {226, 232, 240, 255};
Color textc = {248, 250, 252, 255};
Color selc = {71, 85, 105, 255};
Color hoverc = {51, 65, 85, 200};

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

int mir = 0;
int rot = 0;
bool selDone = false;

vector<int> populationGraph;

unordered_set<cell, cell_hash> livecells;
vector<cell> selection;
vector<state> acts;

int curract = 0;

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

bool buttonHover(Rectangle rect){
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, rect);
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

void resetHm(){
    mir = 0;
    rot = 0;
    selection.clear();
    selDone = false;
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
    resetHm();
}

void loadFile(string filen){
    ifstream file(filen);
    livecells.clear();
    file >> gen >> camera.offset.x >> camera.offset.y >> camera.target.x >> camera.target.y >> camera.zoom >> live;
    for(int i = 0; i < live; i++){
        cell c;
        file >> c.x >> c.y;
        livecells.emplace(c);
    }
    file.close();
}

void currState(unordered_set<cell, cell_hash> livecells, tool currTool, panels currPanel, pattern currPattern, Camera2D camera){
    state curr;
    curr.livecells = livecells;
    curr.currPanel = currPanel;
    curr.currTool = currTool;
    curr.currPattern = currPattern;
    curr.camera.offset = camera.offset;
    curr.camera.target = camera.target;
    curr.camera.zoom = camera.zoom;
    if (curract < acts.size()-1){
        acts.erase(acts.begin() + curract + 1, acts.end());
    }
    acts.emplace_back(curr);
    curract++;
}

void drawPattern(cell c, pattern curr){
    int n = pat[curr].size();
    for(int i = 0; i < n; i++){
        cell nc;
        int x = pat[curr][i].x;
        int y = pat[curr][i].y;
        if(rot == 1){
            int tx = x;
            x = y;
            y = -tx;
        }
        else if(rot == 2){
            x = -x;
            y = -y;
        }
        else if(rot == 3){
            int tx = x;
            x = -y;
            y = tx;
        }
        if(mir == 1){
            x*=-1;
        }
        else if(mir == 2){
            y*=-1;
        }
        nc = {c.x+x, c.y+y};
        livecells.emplace(nc);
        if(curr == selpat) selection.emplace_back(nc);
    }
}

cell formPattern(pattern p){
    cell minn = {(int)4e6, (int)4e6}, maxx = {(int)-4e6, (int)-4e6};
    for(cell c : selection){
        if(c.x < minn.x) minn.x = c.x;
        if(c.x > maxx.x) maxx.x = c.x;
        if(c.y < minn.y) minn.y = c.y;
        if(c.y > maxx.y) maxx.y = c.y;
    }
    int midx = (minn.x+maxx.x)/2, midy = (minn.y+maxx.y)/2;
    cell mid = {midx, midy};
    pat[p].clear();
    for(cell c : selection){
        cell nc = {c.x-mid.x, c.y-mid.y};
        pat[p].emplace_back(nc);
    }
    return mid;
}

void transformSel(){
    cell mid = formPattern(selpat);
    for(cell c : selection) livecells.erase(c);
    selection.clear();
    drawPattern(mid, selpat);
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

    start.livecells.clear();
    start.camera.offset = camera.offset;
    start.camera.target = camera.target;
    start.camera.zoom = 1.0f;
    acts.emplace_back(start);

    Rectangle panel = {screenw-200, 0, 200, screenh};
    Rectangle panelBtn = {screenw-10, screenh-100, 10, 20};

    float btnx = panel.x + 40;
    float btnw = 120, btnh = 40;

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

    int patSel = 0, toolSel = 0;

    vector<Rectangle> btns = {btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9, btn10, btn11};

    int graphx = panel.x+5, graphy = btn4.y;
    int graphw = 190, graphh = 380;

    vector<string> files;

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
        cell curr = getCell();
        DrawRectangle(curr.x*cell_size, curr.y*cell_size, cell_size, cell_size, selc);
        for(int i = starty*cell_size; i < endy*cell_size; i += cell_size){
            DrawLine(startx, i, endx*cell_size, i, lines);
        }
        for(cell c : livecells){
            if(startx <= c.x < endx && starty <= c.y < endy){
                DrawRectangle(c.x*cell_size, c.y*cell_size, cell_size, cell_size, cellc);
            }
        }
        for(cell c : selection){
            if(startx <= c.x < endx && starty <= c.y < endy){
                DrawRectangle(c.x*cell_size, c.y*cell_size, cell_size, cell_size, selc);
            }
        }

        EndMode2D();

        checkBounds();
        if(isPanelOpen){
            DrawRectangleRec(panel, bg);
            if(buttonHover(panelBtn)) DrawRectangleRec(panelBtn, hoverc);
            else DrawRectangleRec(panelBtn, btnc);
            DrawText(">", panelBtn.x+3, panelBtn.y, 20, textc);
            switch(currPanel){
                case def:
                for(int i = 0; i < 5; i++){
                    Color col = btnc;
                    if(buttonHover(btns[i])) col = hoverc;
                    DrawRectangleRec(btns[i], col);
                }
                DrawText("tools", btn1.x+33, btn1.y+10, 20, textc);
                DrawText("patterns", btn2.x+13, btn2.y+10, 20, textc);
                DrawText("stats", btn3.x+33, btn3.y+10, 20, textc);
                DrawText("save", btn4.x+37, btn4.y+10, 20, textc);
                DrawText("load", btn5.x+40, btn5.y+10, 20, textc);
                break;
                case tools:
                for(int i = 0; i < 11; i++){
                    if(i==5) i+=5;
                    Color col = btnc;
                    if(buttonHover(btns[i]) || (toolSel == i)) col = hoverc;
                    DrawRectangleRec(btns[i], col);
                }
                DrawText("brush", btn1.x+30, btn1.y+10, 20, textc);
                DrawText("erase", btn2.x+30, btn2.y+10, 20, textc);
                DrawText("rect", btn3.x+37, btn3.y+10, 20, textc);
                DrawText("line", btn4.x+43, btn4.y+10, 20, textc);
                DrawText("select", btn5.x+28, btn5.y+10, 20, textc);
                DrawText("back", btn11.x+35, btn11.y+10, 20, textc);
                break;
                case patterns:
                for(int i = 0; i < 11; i++){
                    Color col = btnc;
                    if(buttonHover(btns[i]) || patSel == i) col = hoverc;
                    DrawRectangleRec(btns[i], col);
                }
                DrawText("normal", btn1.x+28, btn1.y+10, 20, textc);
                DrawText("glider", btn2.x+33, btn2.y+10, 20, textc);
                DrawText("lwss", btn3.x+1+40, btn3.y+10, 20, textc);
                DrawText("mwss", btn4.x+1+37, btn4.y+10, 20, textc);
                DrawText("hwss", btn5.x+1+37, btn5.y+10, 20, textc);
                DrawText("gosper", btn6.x+28, btn6.y+10, 20, textc);
                DrawText("pulsar", btn7.x+28, btn7.y+10, 20, textc);
                DrawText("pdthlon", btn8.x+23, btn8.y+10, 20, textc);
                DrawText("acorn", btn9.x+31, btn9.y+10, 20, textc);
                DrawText("rpento", btn10.x+27, btn10.y+10, 20, textc);
                DrawText("back", btn11.x+35, btn11.y+10, 20, textc);
                break;
                case stats:
                DrawText(TextFormat("peak\npopulation: %d", peak), btnx-10, btn1.y, 20, textc);
                if (livecells.empty()) DrawText("bounding box:\n0 x 0", btnx-10, btn2.y, 20, textc);
                else DrawText(TextFormat("bounding box:\n%d x %d", maxx-minx+1, maxy-miny+1), btnx-10, btn2.y, 20, textc);
                if(gen) DrawText(TextFormat("avg population: \n%d", total/gen), btnx-10, btn3.y, 20, textc);
                else DrawText("avg population: \n0", btnx-10, btn3.y, 20, textc);
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
                if(buttonHover(btn11)) DrawRectangleRec(btn11, hoverc);
                else DrawRectangleRec(btn11, btnc);
                DrawText("back", btn11.x+35, btn11.y+10, 20, textc);
                break;
                case load:
                for(int i = 0; i < files.size(); i++){
                    Rectangle btn = btns[i];
                    Color col = btnc;
                    if(buttonHover(btn)) col = hoverc;
                    DrawRectangleRec(btn, col);
                    DrawText(files[i].c_str(), btn.x+5, btn.y+10, 20, textc);
                }
                if(buttonHover(btn11)) DrawRectangleRec(btn11, hoverc);
                else DrawRectangleRec(btn11, btnc);
                DrawText("back", btn11.x+35, btn11.y+8, 20, textc);
                break;
            }
        }
        else{
            if(buttonHover(panelBtn)) DrawRectangleRec(panelBtn, hoverc);
            else DrawRectangleRec(panelBtn, btnc);
            DrawText("<", panelBtn.x+1, panelBtn.y, 20, textc);
        }

        DrawText(TextFormat("generations: %d", gen), 10, 10, 20, textc);
        DrawText(TextFormat("live cells: %d", live), 10, 30, 20, textc);
        DrawText(TextFormat("speed: %.2fx", 1-interval), 10, 50, 20, textc);

        EndDrawing();

        wheel = GetMouseWheelMove();
        live = livecells.size();
        peak = (live > peak ? live : peak);

        if(populationGraph.size() > 500){
            populationGraph.erase(populationGraph.begin());
        }

        switch(currTool){
            case brush:
            toolSel = 0;
            break;
            case erase:
            toolSel = 1;
            break;
            case rect:
            toolSel = 2;
            break;
            case line:
            toolSel = 3;
            break;
            case sel:
            toolSel = 4;
        }

        switch(currPattern){
            case nor:
            patSel = 0;
            break;
            case glider:
            patSel = 1;
            break;
            case lwss:
            patSel = 2;
            break;
            case mwss:
            patSel = 3;
            break;
            case hwss:
            patSel = 4;
            break;
            case gosper:
            patSel = 5;
            break;
            case pulsar:
            patSel = 6;
            break;
            case pdthlon:
            patSel = 7;
            break;
            case acorn:
            patSel = 8;
            break;
            case rpento:
            patSel = 9;
            break;
        }

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            if(buttonClick(panelBtn)){
                isPanelOpen = !isPanelOpen;
            }
            else if(isPanelOpen){
                switch(currPanel){
                    case def:
                    if(buttonClick(btn1)){
                        currPanel = tools;
                    }
                    else if(buttonClick(btn2)){
                        currPanel = patterns;
                    }
                    else if(buttonClick(btn3)){
                        currPanel = stats;
                    }
                    else if(buttonClick(btn4) && !running){
                        string filen;
                        for(int n = 1; n < files.size(); n++){
                            filen = "saves/save" + to_string(n) + ".life";
                            if(!filesystem::exists(filen)) break;
                        }
                        ofstream file(filen);
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
                    else if(buttonClick(btn5)){
                        currPanel = load;
                        files.clear();
                        for(auto file : filesystem::directory_iterator("saves")) files.emplace_back(file.path().filename().string());
                        while(files.size() > 10) files.erase(files.begin());
                    }
                    break;
                    case tools:
                    if(buttonClick(btn1)){
                        currTool = brush;
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn2)){
                        currTool = erase;
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn3)){
                        currTool = rect;
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn4)){
                        currTool = line;
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn5)){
                        currTool = sel;
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn11)){
                        currPanel = def;
                    }
                    break;
                    case patterns:
                    if(buttonClick(btn1)){
                        currPattern = nor;
                        resetHm();
                    }
                    else if(buttonClick(btn2)){
                        currPattern = glider;
                        resetHm();
                    }
                    else if(buttonClick(btn3)){
                        currPattern = lwss;
                        resetHm();
                    }
                    else if(buttonClick(btn4)){
                        currPattern = mwss;
                        resetHm();
                    }
                    else if(buttonClick(btn5)){
                        currPattern = hwss;
                        resetHm();
                    }
                    else if(buttonClick(btn6)){
                        currPattern = gosper;
                        resetHm();
                    }
                    else if(buttonClick(btn7)){
                        currPattern = pulsar;
                        resetHm();
                    }
                    else if(buttonClick(btn8)){
                        currPattern = pdthlon;
                        resetHm();
                    }
                    else if(buttonClick(btn9)){
                        currPattern = acorn;
                        resetHm();
                    }
                    else if(buttonClick(btn10)){
                        currPattern = rpento;
                        resetHm();
                    }
                    else if(buttonClick(btn11)){
                        currPanel = def;
                    }
                    break;
                    case stats:
                    if(buttonClick(btn11)){
                        currPanel = def;
                    }
                    break;
                    case load:
                    if(buttonClick(btn11)){
                        currPanel = def;
                    }
                    for(int i = 0; i < 10; i++){
                        if(buttonClick(btns[i])){
                            loadFile("saves/"+files[i]);
                            break;
                        }
                    }
                    mir = 0;
                    rot = 0;
                }
            }
            if((!isPanelOpen || !buttonClick(panel)) && !buttonClick(panelBtn)){
                dragging = true;
                if(currTool == sel){
                    startc = getCell();
                }
                else if(currPattern != nor){
                    currTool = brush;
                    dragging = false;
                    cell c = getCell();
                    drawPattern(c, currPattern);
                }
                else if(currTool == rect || currTool == line){
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

        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
            if(dragging){
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
                    case line:{
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
                    case sel:
                    if(!selDone){
                        selDone = true;
                        for(int x = min(startc.x, endc.x); x <= max(startc.x, endc.x); x++){
                            for(int y = min(startc.y, endc.y); y <= max(startc.y, endc.y); y++){
                                cell c = {x, y};
                                if(livecells.find(c) != livecells.end()){
                                    selection.emplace_back(c);
                                }
                            }
                        }
                    }
                    else{
                        vector<cell> temp = selection;
                        selection.clear();
                        for(cell c : temp) livecells.erase(c);
                        int dx = endc.x-startc.x, dy = endc.y-startc.y;
                        for(int i = 0; i < temp.size(); i++){
                            cell c = temp[i];
                            cell nc = {c.x+dx, c.y+dy};
                            selection.emplace_back(nc);
                            livecells.emplace(nc);
                        }
                    }
                }
                dragging = false;
            }
            currState(livecells, currTool, currPanel, currPattern, camera);
        }

        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)){
            if(!isPanelOpen || !buttonClick(panel)){
                if(selDone){
                    resetHm();
                }
                else{
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
            currState(livecells, currTool, currPanel, currPattern, camera);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            Vector2 delta = GetMouseDelta();
            delta = {delta.x*(-1.0f/camera.zoom), delta.y*(-1.0f/camera.zoom)};
            camera.target.x += delta.x;
            camera.target.y += delta.y;
            currState(livecells, currTool, currPanel, currPattern, camera);
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
            currState(livecells, currTool, currPanel, currPattern, camera);
        }

        if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Z)){
            curract -= (curract > 0);
            state curr = acts[curract];
            livecells = curr.livecells;
            camera = curr.camera;
            currPanel = curr.currPanel;
            currTool = curr.currTool;
            currPattern = curr.currPattern;
            gen = 0;
        }

        if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Y)){
            curract += (curract < acts.size()-1);
            state curr = acts[curract];
            livecells = curr.livecells;
            camera = curr.camera;
            currPanel = curr.currPanel;
            currTool = curr.currTool;
            currPattern = curr.currPattern;
            gen = 0;
        }

        if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C) && selDone){
            cell mid = formPattern(cop);
        }

        if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V)){
            cell c = getCell();
            drawPattern(c, cop);
            currState(livecells, currTool, currPanel, currPattern, camera);
        }

        if((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_X) && selDone){
            cell mid = formPattern(cop);
            for(cell c : selection) livecells.erase(c);
            resetHm();
        }

        if(IsKeyPressed(KEY_SPACE)){
            resetHm();
            running = !running;
        }

        if(currPattern!=nor || selDone){
            if(IsKeyPressed(KEY_X) && !(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))){
                mir = (mir+1 < 3 ? mir+1 : 0);
                if(selDone) transformSel();
                currState(livecells, currTool, currPanel, currPattern, camera);
            }
            else if(IsKeyPressed(KEY_Y)){
                rot = (rot+1 < 4 ? rot+1 : 0);
                if(selDone) transformSel();
                currState(livecells, currTool, currPanel, currPattern, camera);
            }
        }

        if(IsKeyPressed(KEY_DELETE) && selDone){
            for(cell c : selection) livecells.erase(c);
            resetHm();
        }

        if(IsKeyPressed(KEY_C) && !(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))){
            reset();
            camera.zoom = 1.0f;
            camera.offset = {screenw/2.0f, screenh/2.0f};
            camera.target = {rows*cell_size/2.0f, cols*cell_size/2.0f};
            currState(livecells, currTool, currPanel, currPattern, camera);
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
            currState(livecells, currTool, currPanel, currPattern, camera);
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