#include "include/OverlapPanel.hh"
#include "include/Grid.hh"
#include "include/Label.hh"
#include "system/SystemIO.hh"
#include <string>
#include <random>


using namespace easy;



struct EliminatingGame {
private:


    Element grids[10][10] = {};
    OverlapPanel g;
    Label lb;
    std::default_random_engine engine;
    int score = 0;
    constexpr static Color colors[5] = {
        Colors::Red,
        Colors::Blue,
        Colors::Green,
        Colors::Yellow,
        Colors::Purple,
    };

public:

    static constexpr int gsize = 30, margin = 10;

    static constexpr Size size = { 410, 410 };
    
    EliminatingGame() {}

    void InitGrid(OverlapPanel g, Label lb) {
        this->g = g;
        this->lb = lb;
        g->Clear();
        score = 0;
        lb->Text = "Score: 0";
        std::uniform_int_distribution<int> dist(0, 4);
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                Element e = MakeElement();
                e->Margin.Left = (gsize + margin) * j + margin;
                e->Margin.Top = (gsize + margin) * i + margin;
                e->BackgroundColor = colors[dist(engine)];
                e->BorderColor = e->BackgroundColor * 0.8;
                e->BorderThickness = { 3, 3, 3, 3 };
                e->SpecSize = { gsize, gsize };
                e->Enable = false;
                g->Add(e);
                g->BeginAnimation(e,
                                  &_Element::Margin,
                                  e->Margin - Rect { 0, size.Height, 0, 0 },
                                  e->Margin,
                                  1600,
                                  EaseOutBounce,
                                  true);
                grids[i][j] = e;
            }
        }
    }

    bool SwitchGrid(Pos g0, Pos g1) {
        if (g0.X < 0 || g1.X < 0) return false;
        int x0 = (g0.X - margin) / (gsize + margin),
            y0 = (g0.Y - margin) / (gsize + margin),
            x1 = (g1.X - margin) / (gsize + margin),
            y1 = (g1.Y - margin) / (gsize + margin);
        if ((x1 == x0 && y1 == y0) || 
            (y0 != y1 || abs(x0 - x1) > 1) &&
            (x0 != x1 || abs(y0 - y1) > 1)) return false;
        std::swap(grids[y0][x0]->BackgroundColor, grids[y1][x1]->BackgroundColor);
        std::swap(grids[y0][x0]->BorderColor, grids[y1][x1]->BorderColor);
        bool elimTable[10][10] = {};
        int sumScore = Check(x0, y0, elimTable) + Check(x1, y1, elimTable);
        if (sumScore > 0) {
            score += sumScore;
            lb->Text = "Score: " + std::to_string(score);
            Eliminate(elimTable);
            return true;
        }
        std::swap(grids[y0][x0]->BackgroundColor, grids[y1][x1]->BackgroundColor);
        std::swap(grids[y0][x0]->BorderColor, grids[y1][x1]->BorderColor);
        return false;
    }

    bool CheckChainReaction() {
        int contiX[10][10] = {};
        int contiY[10][10] = {};
        for (int i = 0; i < 10; ++i) {
            for (int j = 1; j < 10; ++j) {
                contiX[i][j] = grids[i][j]->BackgroundColor == grids[i][j - 1]->BackgroundColor ? contiX[i][j - 1] + 1 : 0;
            }
        }
        for (int j = 0; j < 10; ++j) {
            for (int i = 1; i < 10; ++i) {
                contiY[i][j] = grids[i][j]->BackgroundColor == grids[i - 1][j]->BackgroundColor ? contiY[i - 1][j] + 1 : 0;
            }
        }
        std::queue<Pos> q;
        bool elimTable[10][10] = {};
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                if (contiX[i][j] > 1 || contiY[i][j] > 1 ||
                    (contiX[i][j] > 0 && contiY[i][j] > 0 && i > 0 && contiX[i - 1][j] > 0))
                {
                    q.push(Pos{ j, i });
                    elimTable[i][j] = true;
                }
            }
        }
        int sumScore = 0;
        if (q.size() > 0) {
            sumScore = BFS(q, elimTable);
            score += sumScore;
            lb->Text = "Score: " + std::to_string(score);
            Eliminate(elimTable);
        }
        return sumScore > 0;
    }

private:

    int BFS(std::queue<Pos>& q, bool elimTable[10][10]) {
        int sumScore = 0;
        while (q.size() > 0) {
            int i = q.front().Y;
            int j = q.front().X;
            q.pop();
            sumScore += 10;
            Color c = grids[i][j]->BackgroundColor;
            if (i > 0 && !elimTable[i - 1][j] && grids[i - 1][j]->BackgroundColor == c) {
                elimTable[i - 1][j] = true;
                q.push(Pos { j, i - 1 });
            }
            if (i + 1 < 10 && !elimTable[i + 1][j] && grids[i + 1][j]->BackgroundColor == c) {
                elimTable[i + 1][j] = true;
                q.push(Pos { j, i + 1 });
            }
            if (j > 0 && !elimTable[i][j - 1] && grids[i][j - 1]->BackgroundColor == c) {
                elimTable[i][j - 1] = true;
                q.push(Pos { j - 1, i });
            }
            if (j + 1 < 10 && !elimTable[i][j + 1] && grids[i][j + 1]->BackgroundColor == c) {
                elimTable[i][j + 1] = true;
                q.push(Pos { j + 1, i });
            }
        }
        return sumScore;
    }

    

    int Check(int x, int y, bool elimTable[10][10]) {
        Color c = grids[y][x]->BackgroundColor;
        int sumScore = 0;
        bool block[5][5] = {};
        for (int i = -2; i < 3; ++i) {
            for (int j = -2; j < 3; ++j) {
                if (x + j >= 0 && x + j < 10 && y + i >= 0 && y + i < 10 && grids[y + i][x + j]->BackgroundColor == c) {
                    block[2 + i][2 + j] = true;
                }
            }
        }
        bool valid = 
            (block[1][2] && block[3][2]) ||
            (block[0][2] && block[1][2]) ||
            (block[3][2] && block[4][2]) ||
            (block[2][0] && block[2][1]) ||
            (block[2][1] && block[2][3]) ||
            (block[2][3] && block[2][4]) ||
            (block[1][1] && block[1][2] && block[2][1]) ||
            (block[3][3] && block[3][2] && block[2][3]) ||
            (block[1][3] && block[1][2] && block[2][3]) ||
            (block[3][1] && block[3][2] && block[2][1]);

        if (valid) {
            std::queue<Pos> q;
            q.push(Pos { x, y });
            elimTable[y][x] = true;
            sumScore = BFS(q, elimTable);
        }

        return sumScore;
    }

    void Eliminate(bool elimTable[10][10]) {
        for (int j = 0; j < 10; ++j) {
            int emptySlot = 0;
            Rect range = { j, 0, j + 1, 10 };
            for (int i = 10 - 1; i >= 0; --i) {
                if (elimTable[i][j]) {
                    if (range.Top > 0) {
                        range.Bottom += emptySlot;
                        for (int k = range.Top; k < range.Bottom; ++k) {
                            g->BeginAnimation(grids[k][j],
                                              &_Element::Margin,
                                              grids[k][j]->Margin,
                                              grids[k][j]->Margin + Rect{ 0, emptySlot * (gsize + margin), 0, 0 },
                                              std::uniform_int_distribution<int>(800, 1200)(engine),
                                              EaseOutBounce,
                                              true);
                        }
                    }
                    ++emptySlot;
                    range.Top = 0;
                    range.Bottom = i;
                } else if (emptySlot > 0) {
                    grids[i + emptySlot][j]->BackgroundColor = grids[i][j]->BackgroundColor;
                    grids[i + emptySlot][j]->BorderColor = grids[i][j]->BorderColor;
                    grids[i + emptySlot][j]->Margin = grids[i][j]->Margin;
                    range.Top = i + emptySlot;
                }
            }
            std::uniform_int_distribution<int> dist(0, 4);
            for (int i = emptySlot - 1; i >= 0; --i) {
                grids[i][j]->BackgroundColor = colors[dist(engine)];
                grids[i][j]->BorderColor = grids[i][j]->BackgroundColor * 0.8;
                grids[i][j]->Margin.Top = (gsize + margin) * (i - emptySlot) + margin;
            }
            if (emptySlot > 0)
            {
                range.Bottom += emptySlot;
                range.Top = 0;
                for (int k = range.Top; k < range.Bottom; ++k) {
                    g->BeginAnimation(grids[k][j],
                                      &_Element::Margin,
                                      grids[k][j]->Margin,
                                      grids[k][j]->Margin + Rect{ 0, emptySlot * (gsize + margin), 0, 0 },
                                      std::uniform_int_distribution<int>(800, 1200)(engine),
                                      EaseOutBounce,
                                      true);
                }
            }
        }
    }


};




int main() {
	Register<Renderer>();

    EliminatingGame game;
    Pos dragged = { -1, -1 };

    Label btn = MakeLabel();
    btn->SpecSize = { 200, 50 };
    btn->VerticalAlignment = VerticalAlignType::Center;
    btn->HorizontalAlignment = HorizontalAlignType::Center;
    btn->BackgroundColor = Color::FromARGB(0xF0F0F0);
    btn->BorderColor = Color::FromARGB(0xC2C2C2);
    btn->BorderThickness = { 4, 4, 4, 4 };
    btn->FontColor = Colors::Brown;
    btn->Text = "Restart";

    Label score = MakeLabel();
    score->BorderColor = Color::FromARGB(0xF0F0F0);
    score->BorderThickness = { 0, 0, 0, 5 };
    score->VerticalAlignment = VerticalAlignType::Center;
    score->HorizontalAlignment = HorizontalAlignType::Center;
    score->Text = "Score: 0";

    Grid panel = MakeGrid({ 0, 0 }, { 0 });
    panel->Set(0, 0, score);
    panel->Set(1, 0, btn);


    OverlapPanel g = MakeOverlapPanel();
    g->Margin = { 20, 20, 20, 20 };
    g->BorderThickness = { 20, 20, 20, 20 };
    g->VerticalAlignment = VerticalAlignType::Center;
    g->BackgroundColor = Color::FromARGB(0xF0F0F0);
    g->BorderColor = Colors::White;

    Grid form = MakeGrid({ 0 }, { 550, 250 });
    form->BackgroundColor = Colors::White;
    form->Set(0, 0, g);
    form->Set(0, 1, panel);

    g->Drag += [&](Element sender, MouseEventArgs args) {
        if (dragged.X < 0 && dragged.Y < 0)
            dragged = args.pos - sender->ActualPos;
    };
    g->Click += [&](Element sender, MouseEventArgs args) {
        game.SwitchGrid(args.pos - sender->ActualPos, dragged);
        dragged = { -1, -1 };
    };

    g->StartAnyAnimation += [form, &dragged](EventArgs args) {
        form->Enable = false;
        dragged = { -1, -1 };
    };

    g->FinishAllAnimation += [form, &game](EventArgs args) {
        if (!game.CheckChainReaction())
            form->Enable = true;
    };

    btn->Click += [g, &game, score](Element sender, MouseEventArgs args) { game.InitGrid(g, score); };

    game.InitGrid(g, score);
	Renderer::MainLoop(form);
}
