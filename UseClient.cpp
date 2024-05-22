#include "Useclient.h"
#include "UseCal.h"
#include "UseUI.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <functional>
#include <thread>
#include <vector>
#include <iostream>

using namespace std;
using namespace sf;

UseClient::UseClient(vector<int> *room_list_set, Shader *bg_shader_set,
                     bool *search_room_complete_set)
    : room_list(room_list_set), bg_shader(bg_shader_set),
      search_room_complete(search_room_complete_set){};
void UseClient::SetUI(UseUI *ui_set) { ui = ui_set; }
void UseClient::SearchServerList() {
  thread search_room(
      [&](vector<int> *room_list, Shader *bg_shader) {
        for (int port = 3000; port < 4000; port++) {
          sf::TcpSocket *socket = new TcpSocket;
          if (socket->connect("localhost", port) == sf::Socket::Done) {
            room_list->push_back(port);
            bg_shader->setUniform("percent", (float)((port - 3000.0) / 2000.0));
            Packet packet;
            packet << "just_searching";
            socket->send(packet);
            socket->disconnect();
          } else {
            delete socket;
            break;
          }
        }
        *search_room_complete = true;
      },
      room_list, bg_shader);
  search_room.detach();
}

void UseClient::ChooseRoom(int room_index_set) { room_index = room_index_set; }

void UseClient::sendID() {
  Packet sending_ID;
  string contents[3] = {"I'm going to tell you my name", "send_id", ID};
  for (int i = 0; i < 3; i++) {
    sending_ID.clear();
    sending_ID << contents[i];
    if (socket->send(sending_ID) == Socket::Done) {
      cout << "send_id successfully " << ID << endl;
    } else {
      cout << "send_id round" << i << " failed" << endl;
    }
  }
}

void UseClient::ReceiveMessage() {
  cout << "start receive message from " << room_index << endl;
  socket = new TcpSocket;
  if (socket->connect("localhost", room_index)) {
    cout << "connect success" << endl;
  } else {
    cout << "connect fail" << endl;
  }
  sendID();

  thread receive_message([&, this]() {
    while (true) {
      Packet packet;
      if (socket->receive(packet) == sf::Socket::Done) {
        string message;
        packet >> message;
        cout << message << endl;
        Packet packetSendBack;
        packetSendBack << "received";
        socket->send(packetSendBack);
        int OwnCard[2];
        if (message == "send_card") {
          packet >> OwnCard[0] >> OwnCard[1];
          cout << "receive card: " << OwnCard[0] << " " << OwnCard[1] << endl;
          ui->AddMoveCard(OwnCard[0], ui->WINDOW_WIDTH / 3,
                          ui->WINDOW_HEIGHT / 3 * 2, 0);
          ui->AddMoveCard(OwnCard[1], ui->WINDOW_WIDTH / 3 * 2,
                          ui->WINDOW_HEIGHT / 3 * 2, 0);
        }
        if (message == "public_card") {
          int new_public_card;
          packet >> new_public_card;
          cout << "public_card" << new_public_card << endl;
          ui->AddNewPublicCard(new_public_card);
        }
        if (message == "your_turn") {
          cout << "it's my turn now" << endl;
          int should_min_fill;
          packet >> should_min_fill;
          ui->SetMyTurn(true, should_min_fill);
        } else if (message == "now_turn") {
          int now_turn;
          packet >> now_turn;
          ui->setTurnsIndex(now_turn);
        }
        if (message == "winner") {
          string winner;
          int winner_score;
          packet >> winner >> winner_score;
          cout << "winner: " << winner << endl;
          ui->SetWinner(winner, winner_score);
        }
        if (message == "lose") {
          int lose_score;
          packet >> lose_score;
          cout << "lose: " << lose_score << endl;
          ui->SetLose(lose_score);
        }
        if (message == "game_start") {
          cout << "game start" << endl;
          Alert("Game Start");
        }
        if (message == "join_game_room") {
          string ID;
          packet >> ID;
          cout << "join_game_room: " << ID << endl;
          Alert(ID + " join the game");
        }
        if (message == "turns_index") {
          int turns_index;
          packet >> turns_index;
          cout << "turns_index: " << turns_index << endl;
          for (int i = 0; i < turns_index; i++) {
            string ID;
            packet >> ID;
            ui->AddPlayersId(ID);
            cout << "new ID " << ID << endl;
            //   IDs.push_back(ID);
          }
          ui->AddPlayersBut();
        }
        if (message == "over_round") {
          cout << "over_round" << endl;
          ui->OverRound();
        }
        if (message == "eio") {
          cout << "eio" << endl;
          ui->SetEio(true);
        }
        if (message == "ready_to_restart") {
          int index;
          packet >> index;
          cout << "ready_to_restart" << index << endl;
        }
      }
    }
  });
  receive_message.detach();
}

void UseClient::SendPacketToServer(Packet packet) {
  if (socket->send(packet) == Socket::Done) {
    cout << "send packet to server successfully" << endl;
  } else {
    cout << "send packet to server failed" << endl;
  }
}

void UseClient::setID(string IDset) { ID = IDset; }

void UseClient::SetAlertInterface(function<void(string)> AlertInterface) {
  Alert = AlertInterface;
}