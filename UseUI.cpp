#include "UseUI.h"
#include "UseCal.h"
#include "UseClient.h"
#include "UseServer.h"
#include <iostream>
#include <vector>

#ifndef USE_CHIP
#include "UseChip.h"
#endif

#ifndef USE_CARD
#include "UseCard.h"
#endif

#ifndef USE_BUTTON
#include "UseButton.h"
#endif

#ifndef USE_ALERT
#include "UseAlert.h"
#endif

using namespace std;
using namespace sf;

UseUI::UseUI(int widthSet, int heightSet, TcpListener *listenerSet)
    : WINDOW_WIDTH(widthSet), WINDOW_HEIGHT(heightSet),
      points(heightSet / 20), listener(listenerSet) {
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

  bg_shader.setUniform("shape_size", sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

  bool cardInit = false;

  if (!tvt_font.loadFromFile("assets/TeyvatBlack-Regular.otf")) {
    std::cout << "Load TeyvatBlack-Regular.ttf failed!" << endl;
  }
  if (!normal_font.loadFromFile("assets/hk4e_zh-cn.ttf")) {
    std::cout << "Load hk4e_zh-cn.ttf failed!" << endl;
  }

  alert = new UseAlert(window, WINDOW_WIDTH, WINDOW_HEIGHT, normal_font);
  alert->addAlert("Alert system init complete");
};
void UseUI::UpdateProgressBar(float percent) {
  bg_shader.setUniform("percent", percent);
  if (percent == 1.0) {
    init_complete = 1;
    bg_shader.setUniform("init_complete", 1);
  }
};
void UseUI::BasicUI(std::function<void()> InsertFn) {
  window.clear();
  InsertFn();
  bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
  window.draw(bg_shape, bg_states);
  alert->printAlert();
  window.display();
};
void UseUI::BasicCoverUI(std::function<void()> InsertFn) {
  window.clear();
  bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
  window.draw(bg_shape, bg_states);
  InsertFn();
  alert->printAlert();
  window.display();
};
void UseUI::RunProgressBar() {
  clock.restart();
  init_complete = 0;
  bg_shader.setUniform("init_complete", 0);
};

void UseUI::InitCards() {
  int index = 0;
  for (CardType type = CardType_Heart; type < CardType_Joker;
       type = (CardType)(type + 1)) {
    for (int i = 1; i <= 13; i++) {
      cards[index++] =
          new UseCard(window, i, type, 0, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 12,
                      WINDOW_HEIGHT / 4, tvt_font, normal_font);

      BasicUI([&]() {
        bg_shader.setUniform(
            "percent",
            smoothstep(0., 1., (((type - CardType_Heart) * 13 + i) / 60.)));
      });
    }
  }
};
void UseUI::InitChips() {
  for (int i = 0; i < 8; i++) {
    chips[i] =
        new UseChip(window, WINDOW_WIDTH / 17 * (i + 1) * 2, WINDOW_HEIGHT / 4,
                    WINDOW_WIDTH / 24, chips_value[i], normal_font);

    BasicUI([&]() {
      bg_shader.setUniform("percent", smoothstep(0., 1., ((i + 53) / 60.)));
    });
  }
};

ServerOrClient UseUI::GameMenu() {
  float circle_center_x = WINDOW_WIDTH / 2;
  float circle_center_y = WINDOW_HEIGHT * 1.2;

  int hoverIndex = -1;
  Clock hoverClock;

  UseButton *button_list[2];
  float button_x = WINDOW_WIDTH / 2;
  float button_y = WINDOW_HEIGHT / 3;

  button_list[0] = new UseButton(window, button_x, button_y, BUTTON_HEIGHT,
                                 "create game room", normal_font);
  button_list[1] = new UseButton(window, button_x, button_y + 70, BUTTON_HEIGHT,
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

void UseUI::RoomOwnerInterface(UseServer &server, int *player_count,
                               bool *GameStart) {
  std::cout << "start room owner interface" << endl;
  float button_x = WINDOW_WIDTH / 2.0;
  float button_y = WINDOW_HEIGHT / 3.0;
  cout << "room owner interface init start" << endl;
  UseButton *start_button = new UseButton(
      window, button_x, button_y, BUTTON_HEIGHT, "start game", normal_font);
  clock.restart();

  cout << "room owner interface init complete" << endl;

  Text text, text2;
  text.setFont(normal_font);
  text.setCharacterSize(24);
  text.setFillColor(Color::Black);
  text.setPosition(10, 10);
  text2.setFont(normal_font);
  text2.setCharacterSize(24);
  text2.setFillColor(Color::Black);
  text2.setPosition(10, 40);

  text2.setString("this room is running on port " + to_string(port));

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

    text.setString("waiting for clients..., now " + to_string(*player_count) +
                   " players");

    window.draw(text);
    window.draw(text2);
    window.display();
  }
  cout << "room owner interface end" << endl;
};
int UseUI::ChooseRoomInterface(UseClient client, bool *search_room_complete,
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

bool UseUI::GameInterface(function<void()> server_init,
                          function<void(Packet)> send_method) {
  Packet packet;
  int btn_height = WINDOW_HEIGHT - BUTTON_HEIGHT - 10;
  UseButton call_button(window, WINDOW_WIDTH / 11 * 2, btn_height,
                        BUTTON_HEIGHT, "call", normal_font);
  UseButton fill_button(window, WINDOW_WIDTH / 11 * 4, btn_height,
                        BUTTON_HEIGHT, "fill", normal_font);
  UseButton dec_button(window, WINDOW_WIDTH / 11 * 6, btn_height, BUTTON_HEIGHT,
                       "decrease", normal_font);
  UseButton give_up_button(window, WINDOW_WIDTH / 11 * 8, btn_height,
                           BUTTON_HEIGHT, "give up", normal_font);
  UseButton over_button(window, WINDOW_WIDTH / 11 * 10, btn_height,
                        BUTTON_HEIGHT, "over", normal_font);

  UseButton restart_game_button(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2,
                                BUTTON_HEIGHT, "restart game", normal_font);
  UseButton exit_button(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 70,
                        BUTTON_HEIGHT, "exit", normal_font);

  bg_shader.setUniform("init_complete", 1);
  clock.restart();

  server_init();

  Text text;
  text.setFont(normal_font);
  text.setString("your turn");
  text.setCharacterSize(points);
  text.setFillColor(Color::Black);
  text.setPosition(10, 10);

  should_fill_text.setFont(normal_font);
  should_fill_text.setCharacterSize(points);
  should_fill_text.setFillColor(Color::Black);
  should_fill_text.setPosition(10, 40);

  now_fill_text.setFont(normal_font);
  now_fill_text.setCharacterSize(points);
  now_fill_text.setFillColor(Color::Black);
  now_fill_text.setPosition(10, 70);

  while (true) {
    window.clear(sf::Color::White);
    bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
    window.draw(bg_shape, bg_states);

    if (show_cards.size() != 0) {
      bool now_card_status_change = false;
      for (int i = 0; i < show_cards.size(); i++) {
        if (card_status_change) {
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
            now_card_status_change = true;
          } else if (cards[show_cards[i]]->x != move_card_dest[i].x ||
                     cards[show_cards[i]]->y != move_card_dest[i].y ||
                     cards[show_cards[i]]->angle != move_card_end_angle[i]) {
            cards[show_cards[i]]->setPos(move_card_dest[i].x,
                                         move_card_dest[i].y);
            cards[show_cards[i]]->setRotation(move_card_end_angle[i]);
          }
        }

        cards[show_cards[i]]->drawCard();
        cards[show_cards[i]]->Hover();
      }
      if (now_card_status_change) {
        card_status_change = true;
      }
    }

    call_button.hover();
    fill_button.hover();
    give_up_button.hover();
    over_button.hover();
    dec_button.hover();

    give_up_button.click();

    lock_guard<mutex> lock(my_turn_mtx);
    if (my_turn) {
      if (call_button.click()) {
        if (should_min_fill != 0) {
          alert->addAlert("Call");
          packet.clear();
          packet << "call";
          now_fill = should_min_fill;
          send_method(packet);
          my_turn = false;
        } else {
          alert->addAlert("You are the first one, you can't call");
        }
      }
      if (fill_button.click()) {
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

      if (to_fill || to_dec) {
        if (dec_button.click()) {
          alert->addAlert("Start decrease");
          to_fill = false;
          to_dec = true;
        }
        for (int i = 0; i < 8; i++) {
          chips[i]->show();
          chips[i]->hover();
          if (chips[i]->click())
            if (to_fill) {
              now_fill += chips_value[i];
              alert->addAlert("Raise $" + to_string(chips_value[i]) + " chips");
              packet.clear();
              packet << "fill" << chips[i]->value;
              send_method(packet);
              now_fill_text.setString("now fill: $" + to_string(now_fill));
            } else if (to_dec) {
              if (now_fill >= chips_value[i]) {
                now_fill -= chips_value[i];
                alert->addAlert("Decrease $" + to_string(chips_value[i]) +
                                " chips");
                packet.clear();
                packet << "fill" << -chips[i]->value;
                send_method(packet);
                now_fill_text.setString("now fill: $" + to_string(now_fill));
              } else {
                alert->addAlert("You don't have enough money to decrease");
              }
            }
        }
        if (over_button.click()) {
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

    for (int i = 0; i < PlayersButs.size(); i++) {
      PlayersButs[i]->show();
    }

    call_button.show();
    fill_button.show();
    give_up_button.show();
    over_button.show();
    dec_button.show();

    if (my_turn) {
      window.draw(text);
      window.draw(should_fill_text);
      window.draw(now_fill_text);
    }

    if (game_over) {
      window.draw(winner_text);
      window.draw(score_text);
      restart_game_button.hover();
      exit_button.hover();
      if (restart_game_button.click()) {
        packet.clear();
        packet << "restart_game";
        send_method(packet);
        game_over = false;
        alert->addAlert("Restart game");
        while (!eio)
          ;
        return true;
      }
      if (exit_button.click()) {
        packet.clear();
        packet << "exit_game";
        send_method(packet);
        game_over = false;
        alert->addAlert("Exit game");
        while (!eio)
          ;

        return false;
      }
      restart_game_button.show();
      exit_button.show();
    }

    alert->printAlert();

    window.display();
  }
}

void UseUI::AddMoveCard(int cardIndex, float x, float y, float angle) {
  show_cards.push_back(cardIndex);
  float move_time = (x + y) / 700.;
  move_card_start_time.push_back(clock.getElapsedTime().asSeconds());
  move_card_time.push_back(move_time);
  move_card_dest.push_back(Vector2i(x, y));
  move_card_end_angle.push_back(angle);
  card_status_change = true;
};

void UseUI::AddNewPublicCard(int cardIndex) {
  cout << "to add new public card" << endl;
  AddMoveCard(cardIndex, WINDOW_WIDTH / 8 * show_cards.size(),
              WINDOW_HEIGHT / 3, 0);
  alert->addAlert("New public card: " + TransformCardIndexToString(cardIndex));
};

void UseUI::SetMyTurn(bool my_turn_set, int should_min_fill_set) {
  lock_guard<mutex> lock(my_turn_mtx);
  my_turn = my_turn_set;
  cout << "set my_turn: " << my_turn << endl;
  alert->addAlert("Your turn");
  should_min_fill = should_min_fill_set;
  should_fill_text.setString("should fill: $" + to_string(should_min_fill));
  now_fill_text.setString("now fill: $" + to_string(now_fill));
};

void UseUI::InputContent(string *content) {
  UseButton *ID_getter, *Confirm_btn;
  ID_getter = new UseButton(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 3,
                            BUTTON_HEIGHT, "input your ID", normal_font);
  Confirm_btn = new UseButton(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 3 + 75,
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
        own_ID = *content;
        return;
        ;
      }
    }
  }
};

void UseUI::AlertInterface(string alertCon) {
  // this->alert->addAlert(alertCon);
}

void UseUI::AddPlayersId(string ID) {
  cout << "add player " << ID << endl;
  IDs.push_back(ID);
};

void UseUI::AddPlayersBut() {
  int num = IDs.size();
  for (int i = 0; i < num; i++) {
    float x = WINDOW_WIDTH / (num * 2 + 1) * (i * 2 + 1);
    float y = WINDOW_HEIGHT / 5;
    PlayersButs.push_back(
        new UseButton(window, x, y, BUTTON_HEIGHT, IDs[i], normal_font));
  }
  PlayersButs.push_back(new UseButton(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT,
                                      BUTTON_HEIGHT, "test_bug", normal_font));
}

void UseUI::setTurnsIndex(int turnsIndexSet) {
  cout << turn_ID << "'s turn" << endl;
  PlayersButs[turn_ID]->setHoverState(false);
  turn_ID = turnsIndexSet;
  PlayersButs[turn_ID]->setHoverState(true);
}

void UseUI::OverRound() {
  alert->addAlert("Over round");
  now_fill = 0;
  my_turn = false;
}

void UseUI::SetWinner(string winner, int win_score) {
  cout << " set winner winner is " << winner << " win score: " << win_score
       << endl;
  winner_text.setFont(normal_font);
  winner_text.setCharacterSize(24);
  winner_text.setFillColor(Color::Black);
  string content;
  if (winner == own_ID) {
    content = "You win, score: " + to_string(win_score);
    game_over = true;
  } else {
    content = "Winner is " + winner + " win score: " + to_string(win_score);
  }

  winner_text.setString(content);
  winner_text.setPosition(WINDOW_WIDTH / 2 -
                              winner_text.getLocalBounds().width / 2,
                          WINDOW_HEIGHT / 3);

  PlayersButs[turn_ID]->setHoverState(false);
}

void UseUI::SetLose(int lose_score) {
  score_text.setFont(normal_font);
  score_text.setCharacterSize(24);
  score_text.setFillColor(Color::Black);
  string content = "You lose, score: " + to_string(lose_score);
  score_text.setString(content);
  score_text.setPosition(WINDOW_WIDTH / 2 -
                             score_text.getLocalBounds().width / 2,
                         WINDOW_HEIGHT / 3 + 50);
  game_over = true;
}

void UseUI::SetEio(bool eioSet) {
  eio = eioSet;
  alert->addAlert("every one is ok");
};

void UseUI::SetReadyToRestart(int index) {
  alert->addAlert(to_string(index) + "ready to restart");
  PlayersButs[index]->setHoverState(true);
}

void UseUI::SetGameStart() { alert->addAlert("Game Start"); }

void UseUI::SetPlayerExit(int index) {
  alert->addAlert("Player " + to_string(index) + " exit");
  PlayersButs.erase(PlayersButs.begin() + index);
  IDs.erase(IDs.begin() + index);
  for (int i = 0; i < PlayersButs.size(); i++) {
    PlayersButs[i]->moveTo(WINDOW_WIDTH / (IDs.size() * 2 + 1) * (i * 2 + 1),
                           WINDOW_HEIGHT / 5);
  }
}
