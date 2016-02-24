#include <iostream>
#include <vector>
#include <deque>
#include <ctime>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

using namespace std;

typedef pair<int, int> pii;

class Game : public sf::RenderWindow {
private:
    const int GAMEOVER = 0;
    const int PLAYING = 1;
    const int PAUSE = 2;

    const int EMPTY = 0;
    const int STONE = 1;
    const int BODY = 2;
    const int FOOD = 3;

    const int UP = 0;
    const int DOWN = 1;
    const int LEFT = 2;
    const int RIGHT = 3;

    const int dr[4] = {-1, +1, 0, 0};
    const int dc[4] = {0, 0, -1, +1};

    int game_state;

    int N; // width of the map
    int L; // length of the snake
    int dir; // direction of the snake
    vector<vector<int>> data;
    deque<pii> snake;

    float unit_w;
    float unit_h;
    sf::RectangleShape stone_view;
    sf::RectangleShape body_view;
    sf::RectangleShape food_view;

    const float time_unit = 0.3f; // seconds
    sf::Clock main_timer;

    sf::Font font;
    sf::Text msg1;
    sf::Text msg2;

    int randint(int a, int b) {
        return rand() % (b - a + 1) + a;
    }

    void generate_thing(int thing) {
        vector<pii> v;
        for (int r = 0; r < N; r++)
            for (int c = 0; c < N; c++)
                if (data[r][c] == EMPTY)
                    v.push_back(pii(r, c));

        int idx = randint(0, v.size() - 1);
        data[v[idx].first][v[idx].second] = thing;
    }

    void restart() {
        // clean data
        data.clear();
        snake.clear();

        game_state = PLAYING;

        // construct data
        for (int i = 0; i < N; i++) {
            data.push_back(vector<int>(N, EMPTY));
        }

        // find some random, non-overlapping position
        vector<pii> v;
        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                v.push_back(pii(r, c));
            }
        }
        for (int i = 1; i < N * N; i++) { // shuffle
            int idx = randint(0, i);
            swap(v[i], v[idx]);
        }

        // init stones
        int stones_num = randint(0, 3);
        for (int i = 0; i < stones_num; i++) {
            int r = v.back().first;
            int c = v.back().second;
            data[r][c] = STONE;
            v.pop_back();
        }

        // init snake
        this->dir = randint(0, 3);
        int snake_init_r = v.back().first;
        int snake_init_c = v.back().second;
        data[snake_init_r][snake_init_c] = BODY;
        snake.push_back(pii(snake_init_r, snake_init_c));
        v.pop_back();

        // init first food
        this->generate_thing(FOOD);
    }

    void init() {
        this->restart();
        // RectangleShapes
        sf::Vector2f unit(unit_w, unit_h);
        stone_view = sf::RectangleShape(unit);
        stone_view.setFillColor(sf::Color(255, 81, 68));
        body_view = sf::RectangleShape(unit);
        body_view.setFillColor(sf::Color(0, 204, 255));
        food_view = sf::RectangleShape(unit);
        food_view.setFillColor(sf::Color(19, 169, 136));

        // font & msg
        if (!font.loadFromFile("Inconsolata-Bold.ttf")) {
            puts("fonts loading error!");
            this->close();
        }
        msg1.setFont(font);
        msg1.setColor(sf::Color::White);
        msg1.setCharacterSize(50);
        msg1.setPosition(80, 100);
        msg2.setFont(font);
        msg2.setColor(sf::Color::White);
        msg2.setCharacterSize(25);
        msg2.setString("Press <Enter> to Replay");
        msg2.setPosition(60, 250);
    }

    void update_msg(const string s) {
        msg1.setString(s);
    }

    void timers_start() {
        main_timer.restart();
    }

    pii next_pos(pii pos, int d) {
        int nr = pos.first + dr[d];
        int nc = pos.second + dc[d];

        if (nr == N) nr = 0;
        if (nc == N) nc = 0;
        if (nr == -1) nr = N - 1;
        if (nc == -1) nc = N - 1;

        return pii(nr, nc);
    }

    void snake_move() {
        pii np = this->next_pos(snake.front(), this->dir);
        int head_nr = np.first;
        int head_nc = np.second;

        if (data[head_nr][head_nc] == EMPTY) {
            int tail_r = snake.back().first;
            int tail_c = snake.back().second;
            data[tail_r][tail_c] = EMPTY;
            snake.pop_back();

            data[head_nr][head_nc] = BODY;
            snake.push_front(pii(head_nr, head_nc));
        }
        else if (data[head_nr][head_nc] == FOOD) {
            data[head_nr][head_nc] = BODY;
            snake.push_front(pii(head_nr, head_nc));
            generate_thing(FOOD);

            if (randint(0, 2) == 0) { // 1/3 probability
                generate_thing(STONE);
            }
        }
        else if (data[head_nr][head_nc] == BODY) {
            game_state = GAMEOVER;
            update_msg(" GAMEOVER\nLength = " + to_string(this->snake.size()));
            printf("self at <%d, %d>\n", head_nr, head_nc);
        }
        else if (data[head_nr][head_nc] == STONE) {
            game_state = GAMEOVER;
            printf("game_state : %d\n", game_state);
            update_msg(" GAMEOVER\nLength = " + to_string(this->snake.size()));
            printf("stone at <%d, %d>\n", head_nr, head_nc);
        }
    }

public:
    Game() {}

    Game(sf::VideoMode mode, const string title, int n) : sf::RenderWindow(mode, title) {
        this->N = n;
        sf::Vector2u window_size = this->getSize();
        this->unit_w = (float) window_size.x / N;
        this->unit_h = (float) window_size.y / N;

        this->setVerticalSyncEnabled(true);;
        this->init();
        this->timers_start();
    }

    void process_event() {
        sf::Event event;
        while (this->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                this->close();

            if (event.type == sf::Event::LostFocus) {
                game_state = PAUSE;
            }

            if (event.type == sf::Event::GainedFocus) {
                game_state = PLAYING;
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    if (snake.size() == 1 || next_pos(snake[1], DOWN) != snake[0])
                        this->dir = UP;
                }
                if (event.key.code == sf::Keyboard::Down) {
                    if (snake.size() == 1 || next_pos(snake[1], UP) != snake[0])
                        this->dir = DOWN;
                }
                if (event.key.code == sf::Keyboard::Left) {
                    if (snake.size() == 1 || next_pos(snake[1], RIGHT) != snake[0])
                        this->dir = LEFT;
                }
                if (event.key.code == sf::Keyboard::Right) {
                    if (snake.size() == 1 || next_pos(snake[1], LEFT) != snake[0])
                        this->dir = RIGHT;
                }
            }

            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Return && game_state == GAMEOVER) {
                    this->restart();
                    this->timers_start();
                }
            }
        }
    }

    void render() {
        this->clear(sf::Color(50, 50, 50));

        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                sf::RectangleShape grid;
                if (data[r][c] == STONE)
                    grid = this->stone_view;
                if (data[r][c] == BODY)
                    grid = this->body_view;
                if (data[r][c] == FOOD)
                    grid = this->food_view;

                grid.setPosition(sf::Vector2f(c * unit_w, r * unit_h));
                this->draw(grid);
            }
        }

        if (game_state == GAMEOVER) {
            this->draw(this->msg2);
            this->draw(this->msg1);
        }

        this->display();
    }

    void animation() {
        if (main_timer.getElapsedTime().asSeconds() > time_unit) {
            this->snake_move();
            main_timer.restart();
        }
    }

    void main_loop() {
        while (this->isOpen()) {
            this->process_event();

            if (game_state == PLAYING)
                this->animation();

            this->render();
        }
    }
};

int main() {
    srand(time(NULL));
	Game window(sf::VideoMode(400, 400), "Snake", 15);
    window.main_loop();
    return 0;
}
