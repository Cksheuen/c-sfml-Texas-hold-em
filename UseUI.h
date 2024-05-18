#ifndef USE_UI
#define USE_UI

#define BUTTON_HEIGHT WINDOW_HEIGHT / 5

#include "UseAlert.h"
#include "UseCal.h"

enum ServerOrClient { Server, Client };

class UseUI {
private:
  TcpListener *listener;
  RenderWindow window;
  Clock clock;
  RenderStates bg_states;
  RectangleShape bg_shape;

  Font tvt_font, normal_font;

  UseCard *cards[52];
  UseChip *chips[8];

  UseAlert *alert;

  vector<float> move_card_start_time, move_card_end_angle, move_card_time;
  vector<Vector2i> move_card_dest;

  bool my_turn = false;

  mutex my_turn_mtx;

  int WINDOW_WIDTH, WINDOW_HEIGHT;
  int init_complete = 0;
  bool to_fill = false;
  TcpSocket *socket = new TcpSocket;
  int join_room_index = -1;

  vector<UseButton *> PlayersButs;

  int turn_ID, should_min_fill = 0, now_fill = 0;

  Text should_fill_text, now_fill_text;

public:
  vector<int> show_cards;
  Shader bg_shader;

  UseUI(int widthSet, int heightSet, TcpListener *listenerSet)
      : WINDOW_WIDTH(widthSet), WINDOW_HEIGHT(heightSet),
        listener(listenerSet) {
    window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game Project");
    bg_shape.setSize(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    bg_shape.setPosition(0, 0);

    string VertexShaderPath = "shaders/Bg/bg.vert";
    string FragmentShaderPath = "shaders/Bg/bg.frag";

    bool state = bg_shader.loadFromFile(VertexShaderPath, FragmentShaderPath);
    if (state) {
      std::cout << "Shader loaded successfully!" << std::endl;
    } else {
      std::cout << "Error: " << state << std::endl;
    }
    bg_states.shader = &bg_shader;

    bg_shader.setUniform("shape_size",
                         sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

    bool cardInit = false;

    if (!tvt_font.loadFromFile("assets/TeyvatBlack-Regular.otf")) {
      std::cout << "Load TeyvatBlack-Regular.ttf failed!" << endl;
    }
    if (!normal_font.loadFromFile("assets/hk4e_zh-cn.ttf")) {
      std::cout << "Load hk4e_zh-cn.ttf failed!" << endl;
    }

    alert = new UseAlert(window, WINDOW_WIDTH, WINDOW_HEIGHT, normal_font);
  };
  void UpdateProgressBar(float percent) {
    bg_shader.setUniform("percent", percent);
    if (percent == 1.0) {
      init_complete = 1;
      bg_shader.setUniform("init_complete", 1);
    }
  };
  void BasicUI(std::function<void()> InsertFn) {
    window.clear();
    InsertFn();
    bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
    window.draw(bg_shape, bg_states);
    window.display();
  };
  void BasicCoverUI(std::function<void()> InsertFn) {
    window.clear();
    bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
    window.draw(bg_shape, bg_states);
    InsertFn();
    alert->printAlert();
    window.display();
  };
  void RunProgressBar() {
    clock.restart();
    init_complete = 0;
    bg_shader.setUniform("init_complete", 0);
  };

  void InitCards() {
    int index = 0;
    for (CardType type = CardType_Heart; type < CardType_Joker;
         type = (CardType)(type + 1)) {
      for (int i = 1; i <= 13; i++) {
        cards[index++] =
            new UseCard(window, i, type, 0, WINDOW_HEIGHT / 2,
                        WINDOW_HEIGHT / 12, WINDOW_HEIGHT / 4, tvt_font);

        BasicUI([&]() {
          bg_shader.setUniform(
              "percent",
              smoothstep(0., 1., (((type - CardType_Heart) * 13 + i) / 60.)));
        });
      }
    }
  };
  void InitChips() {
    for (int i = 0; i < 8; i++) {
      chips[i] = new UseChip(window, WINDOW_WIDTH / 17 * (i + 1) * 2,
                             WINDOW_HEIGHT / 4, WINDOW_WIDTH / 24,
                             chips_value[i], normal_font);

      BasicUI([&]() {
        bg_shader.setUniform("percent", smoothstep(0., 1., ((i + 53) / 60.)));
      });
    }
  };

  ServerOrClient GameMenu() {
    float circle_center_x = WINDOW_WIDTH / 2;
    float circle_center_y = WINDOW_HEIGHT * 1.2;

    int hoverIndex = -1;
    Clock hoverClock;

    UseButton *button_list[2];
    float button_x = WINDOW_WIDTH / 2;
    float button_y = WINDOW_HEIGHT / 3;

    button_list[0] = new UseButton(window, button_x, button_y, BUTTON_HEIGHT,
                                   "create game room", normal_font);
    button_list[1] =
        new UseButton(window, button_x, button_y + 70, BUTTON_HEIGHT,
                      "join game room", normal_font);

    bg_shader.setUniform("init_complete", 1);
    clock.restart();

    while (true) {
      window.clear(sf::Color::White);
      bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
      window.draw(bg_shape, bg_states);

      for (int i = 0; i < 52; i++) {
        if (clock.getElapsedTime().asSeconds() <= 1.) {
          float start_theta =
              atan2f(-WINDOW_HEIGHT / 2., -WINDOW_WIDTH / 2.) / 3.14 * 180;
          float end_theta = (abs(start_theta) - 90) / 52 * 2. * i;
          float now_theta =
              smoothstep(float(0.), 1., clock.getElapsedTime().asSeconds()) *
              end_theta;
          float x =
              circle_center_x +
              cosf((start_theta + now_theta) / 180 * 3.14) * WINDOW_WIDTH / 1;
          float y =
              circle_center_y +
              sinf((start_theta + now_theta) / 180 * 3.14) * WINDOW_HEIGHT / 2;
          cards[i]->setPos(x, y);
          cards[i]->setRotation(start_theta + now_theta - 90);
        }
        cards[i]->drawCard();
      }
      for (int i = 51; i >= 0; i--) {
        if (cards[i]->Hover()) {
          if (i != hoverIndex) {
            if (hoverIndex != -1)
              cards[hoverIndex]->scale(1, 1);
            hoverClock.restart();
            hoverIndex = i;
          } else {
            float scale =
                smoothstep(0., .01, hoverClock.getElapsedTime().asSeconds()) *
                    .2 +
                1.;
            cards[i]->scale(scale, scale);
          }
          break;
        } else if (hoverIndex == i) {
          hoverIndex = -1;
          cards[i]->scale(1, 1);
        }
      }

      for (int i = 0; i < 8; i++) {
        chips[i]->show();
      }

      for (int i = 0; i < 2; i++) {
        button_list[i]->hover();
        button_list[i]->show();
      }

      if (button_list[0]->click()) {
        int port = 3000;
        while (listener->listen(port) != Socket::Done && port < 4000) {
          port++;
        }
        std::cout << "create game room, the room is on the local port: " << port
                  << endl;
        return Server;
        break;
      }
      if (button_list[1]->click()) {
        std::cout << "join game room" << endl;
        return Client;
        break;
      }

      window.display();
    }
  };

  void RoomOwnerInterface(UseServer &server, int *player_count,
                          bool *GameStart) {
    std::cout << "start room owner interface" << endl;
    float button_x = WINDOW_WIDTH / 2;
    float button_y = WINDOW_HEIGHT / 3;
    cout << "room owner interface init start" << endl;
    UseButton *start_button = new UseButton(
        window, button_x, button_y, BUTTON_HEIGHT, "start game", normal_font);
    clock.restart();

    cout << "room owner interface init complete" << endl;

    while (!*GameStart) {
      // server.ReceiveMessageInWhile();
      window.clear(sf::Color::White);

      bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
      window.draw(bg_shape, bg_states);

      start_button->hover();
      if (start_button->click()) {
        *GameStart = true;
        std::cout << "game_start" << endl;
        Packet packet;
        packet << "game_start";
        server.SendToAllClients(packet);
      }
      start_button->show();

      Text text;
      text.setFont(normal_font);
      text.setString("waiting for clients..., now " + to_string(*player_count) +
                     " players");
      text.setCharacterSize(24);
      text.setFillColor(Color::Black);
      text.setPosition(10, 10);
      window.draw(text);

      window.display();
    }
    cout << "room owner interface end" << endl;
  };
  int ChooseRoomInterface(UseClient client, bool *search_room_complete,
                          vector<int> &room_list) {

    float button_x = WINDOW_WIDTH / 2;
    float button_y = WINDOW_HEIGHT / 3;

    RunProgressBar();

    while (!*search_room_complete) {
      bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
      window.draw(bg_shape, bg_states);
      window.display();
    }

    vector<UseButton *> room_button_list;

    for (int i = 0; i < room_list.size(); i++) {
      room_button_list.push_back(
          new UseButton(window, button_x, button_y + i * 70, BUTTON_HEIGHT,
                        "room " + to_string(room_list[i]), normal_font));

      BasicUI([&]() {
        bg_shader.setUniform("percent",
                             (float)(0.5 + (i + 1) / room_list.size()));
      });
    }

    bg_shader.setUniform("init_complete", 1);
    clock.restart();

    int room_index = -1;
    bool GameStart = false;
    while (!GameStart) {
      window.clear(sf::Color::White);
      bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
      window.draw(bg_shape, bg_states);

      for (int i = 0; i < room_button_list.size(); i++) {
        room_button_list[i]->hover();
        room_button_list[i]->show();
        if (room_button_list[i]->click()) {
          return room_list[i];
          GameStart = true;
          break;
        }
      }

      window.display();
    }
  };

  void GameInterface(function<void()> server_init,
                     function<void(Packet)> send_method) {
    Packet packet;
    int btn_height = WINDOW_HEIGHT / 3 * 2;
    UseButton call_button(window, WINDOW_WIDTH / 11 * 2, btn_height,
                          BUTTON_HEIGHT, "call", normal_font);
    UseButton fill_button(window, WINDOW_WIDTH / 11 * 4, btn_height,
                          BUTTON_HEIGHT, "fill", normal_font);
    UseButton back_button(window, WINDOW_WIDTH / 11 * 6, btn_height,
                          BUTTON_HEIGHT, "back", normal_font);
    UseButton give_up_button(window, WINDOW_WIDTH / 11 * 8, btn_height,
                             BUTTON_HEIGHT, "give up", normal_font);
    UseButton over_button(window, WINDOW_WIDTH / 11 * 10, btn_height,
                          BUTTON_HEIGHT, "over", normal_font);

    bg_shader.setUniform("init_complete", 1);
    clock.restart();

    server_init();

    Text text;
    text.setFont(normal_font);
    text.setString("your turn");
    text.setCharacterSize(24);
    text.setFillColor(Color::Black);
    text.setPosition(10, 10);

    should_fill_text.setFont(normal_font);
    should_fill_text.setCharacterSize(24);
    should_fill_text.setFillColor(Color::Black);
    should_fill_text.setPosition(10, 40);

    now_fill_text.setFont(normal_font);
    now_fill_text.setCharacterSize(24);
    now_fill_text.setFillColor(Color::Black);
    now_fill_text.setPosition(10, 70);

    while (true) {
      window.clear(sf::Color::White);
      bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
      window.draw(bg_shape, bg_states);

      call_button.hover();
      fill_button.hover();
      give_up_button.hover();
      back_button.hover();
      over_button.hover();

      {
        lock_guard<mutex> lock(my_turn_mtx);
        if (my_turn) {
          if (call_button.click()) {
            if (should_min_fill != 0) {
              alert->addAlert("Call");
              packet.clear();
              packet << "call";
              send_method(packet);
              my_turn = false;
            } else {
              alert->addAlert("You are the first one, you can't call");
            }
          }
          if (fill_button.click()) {
            /* packet.clear();
            packet << "fill";
            send_method(packet); */

            alert->addAlert("Start fill");
            to_fill = true;
          }
          if (give_up_button.click()) {
            packet.clear();
            packet << "give_up";
            alert->addAlert("Give up");
            send_method(packet);
            my_turn = false;
          }

          if (to_fill) {
            for (int i = 0; i < 8; i++) {
              chips[i]->show();
              chips[i]->hover();
              if (chips[i]->click()) {
                now_fill += chips_value[i];
                alert->addAlert("Raise $" + to_string(chips_value[i]) +
                                " chips");
                packet.clear();
                packet << "fill" << chips[i]->value;
                send_method(packet);
                now_fill_text.setString("now fill: $" + to_string(now_fill));
              }
            }
            back_button.show();
            back_button.hover();
            if (back_button.click()) {
              if (now_fill >= should_min_fill) {
                to_fill = false;
                packet.clear();
                packet << "over_turn";
                send_method(packet);
                std::cout << "over_turn" << endl;
                my_turn = false;
                alert->addAlert("Over turn");
              } else {
                alert->addAlert("You should fill at least $" +
                                to_string(should_min_fill));
              }
            }
          }
        }
      }

      for (int i = 0; i < PlayersButs.size(); i++) {
        PlayersButs[i]->show();
      }

      if (show_cards.size() != 0) {
        for (int i = 0; i < show_cards.size(); i++) {
          if (clock.getElapsedTime().asSeconds() <
              move_card_start_time[i] + move_card_time[i]) {
            float time =
                clock.getElapsedTime().asSeconds() - move_card_start_time[i];
            float dx =
                smoothstep(0, move_card_time[i], time) * move_card_dest[i].x;
            float dy =
                smoothstep(0, move_card_time[i], time) * move_card_dest[i].y;
            float angle = smoothstep(0, 1, clock.getElapsedTime().asSeconds()) *
                          move_card_end_angle[i];
            cards[show_cards[i]]->setPos(dx, dy);
            cards[show_cards[i]]->setRotation(angle);
          } else if (cards[show_cards[i]]->x != move_card_dest[i].x ||
                     cards[show_cards[i]]->y != move_card_dest[i].y ||
                     cards[show_cards[i]]->angle != move_card_end_angle[i]) {
            cards[show_cards[i]]->setPos(move_card_dest[i].x,
                                         move_card_dest[i].y);
            cards[show_cards[i]]->setRotation(move_card_end_angle[i]);
          }
          cards[show_cards[i]]->drawCard();
        }
      }

      call_button.show();
      fill_button.show();
      give_up_button.show();
      back_button.show();
      over_button.show();

      if (my_turn) {
        window.draw(text);
        window.draw(should_fill_text);
        window.draw(now_fill_text);
      }

      alert->printAlert();

      window.display();
    }
  }

  void AddMoveCard(int cardIndex, float x, float y, float angle) {
    show_cards.push_back(cardIndex);
    float move_time = (x + y) / 700.;
    move_card_start_time.push_back(clock.getElapsedTime().asSeconds());
    move_card_time.push_back(move_time);
    move_card_dest.push_back(Vector2i(x, y));
    move_card_end_angle.push_back(angle);
  };

  void AddNewPublicCard(int cardIndex) {
    cout << "to add new public card" << endl;
    AddMoveCard(cardIndex, WINDOW_WIDTH / 8 * show_cards.size(),
                WINDOW_HEIGHT / 3, 0);
    alert->addAlert("New public card: " +
                    TransformCardIndexToString(cardIndex));
  };

  void SetMyTurn(bool my_turn_set, int should_min_fill_set) {
    lock_guard<mutex> lock(my_turn_mtx);
    my_turn = my_turn_set;
    cout << "set my_turn: " << my_turn << endl;
    alert->addAlert("Your turn");
    should_min_fill = should_min_fill_set;
    now_fill = 0;
    should_fill_text.setString("should fill: $" + to_string(should_min_fill));
    now_fill_text.setString("now fill: $" + to_string(now_fill));
  };

  void InputContent(string *content) {
    UseButton *ID_getter, *Confirm_btn;
    ID_getter = new UseButton(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 3,
                              BUTTON_HEIGHT, "input your ID", normal_font);
    Confirm_btn =
        new UseButton(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 3 + 75,
                      BUTTON_HEIGHT, "confirm", normal_font);
    Event event;

    while (true) {
      while (window.pollEvent(event)) {
        if (event.type == Event::TextEntered) {
          if (event.text.unicode < 128 && event.text.unicode != 8) {
            *content += static_cast<char>(event.text.unicode);
            ID_getter->setString(*content);
          } else if (event.text.unicode == 8) {
            if (!content->empty()) {
              content->pop_back();
              ID_getter->setString(*content);
            }
          }
        }
      }
      BasicCoverUI([&] {
        Confirm_btn->hover();
        Confirm_btn->show();

        ID_getter->show();
      });
      if (Confirm_btn->click()) {
        if (content->empty()) {
          alert->addAlert("ID can't be empty");
        } else {
          return;
          ;
        }
      }
    }
  };

  void AlertInterface(string alert) { this->alert->addAlert(alert); }

  void AddPlayersBut(string ID) {
    cout << "add player " << ID << endl;
    PlayersButs.push_back(new UseButton(window, WINDOW_WIDTH / 2,
                                        WINDOW_HEIGHT / 10, BUTTON_HEIGHT, ID,
                                        normal_font));
    for (int i = 0; i < PlayersButs.size(); i++) {
      PlayersButs[i]->moveTo(WINDOW_WIDTH / (PlayersButs.size() + 1) * (i + 1),
                             WINDOW_HEIGHT / 5);
      cout << i << " move to "
           << WINDOW_WIDTH / (PlayersButs.size() + 1) * (i + 1) << " "
           << WINDOW_HEIGHT / 5 << endl;
    }
  };

  void setTurnsIndex(int turnsIndexSet) {
    cout << turn_ID << "'s turn" << endl;
    PlayersButs[turn_ID]->setHoverState(false);
    turn_ID = turnsIndexSet;
    PlayersButs[turn_ID]->setHoverState(true);
  }
};

#endif