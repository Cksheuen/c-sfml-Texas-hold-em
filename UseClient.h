#ifndef USE_CLIENT
#define USE_CLIENT

#include <SFML/Network.hpp>
#include <functional>
#include <thread>

using namespace std;
using namespace sf;

class UseClient {
private:
  Shader *bg_shader;
  vector<int> *room_list;
  bool *search_room_complete;
  int room_index;
  TcpSocket *socket;

  string ID;

  function<void(string)> Alert;

public:
  UseClient(vector<int> *room_list_set, Shader *bg_shader_set,
            bool *search_room_complete_set)
      : room_list(room_list_set), bg_shader(bg_shader_set),
        search_room_complete(search_room_complete_set){};
  void SearchServerList() {
    thread search_room(
        [&](vector<int> *room_list, Shader *bg_shader) {
          for (int port = 3000; port < 4000; port++) {
            sf::TcpSocket *socket = new TcpSocket;
            if (socket->connect("localhost", port) == sf::Socket::Done) {
              room_list->push_back(port);
              bg_shader->setUniform("percent",
                                    (float)((port - 3000.0) / 2000.0));
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

  void ChooseRoom(int room_index_set) { room_index = room_index_set; }

  void sendID() {
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

  void ReceiveMessage(function<void(int, int)> MoveCardFn,
                      function<void(int)> AddNewPublicCardFn,
                      function<void(bool, int)> SetMyTurn,
                      function<void(string)> AddPlayersIds,
                      function<void(int)> SetTurnsIndex,
                      function<void()> OverRoundFn,
                      function<void(string, int)> SetWinner,
                      function<void(int)> SetLose,
                      function<void()> AddPlayersBut) {
    cout << "start receive message from " << room_index << endl;
    socket = new TcpSocket;
    if (socket->connect("localhost", room_index)) {
      cout << "connect success" << endl;
    } else {
      cout << "connect fail" << endl;
    }
    sendID();

    thread receive_message(
        [&, MoveCardFn = move(MoveCardFn),
         AddNewPublicCardFn = move(AddNewPublicCardFn),
         SetMyTurn = move(SetMyTurn), AddPlayersIds = move(AddPlayersIds),
         SetTurnsIndex = move(SetTurnsIndex), OverRoundFn = move(OverRoundFn),
         SetWinner = move(SetWinner), SetLose = move(SetLose),
         AddPlayersBut = move(AddPlayersBut), this]() {
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
                cout << "receive card: " << OwnCard[0] << " " << OwnCard[1]
                     << endl;
                MoveCardFn(OwnCard[0], OwnCard[1]);
              }
              if (message == "public_card") {
                int new_public_card;
                packet >> new_public_card;
                cout << "public_card" << new_public_card << endl;
                AddNewPublicCardFn(new_public_card);
              }
              if (message == "your_turn") {
                cout << "it's my turn now" << endl;
                int should_min_fill;
                packet >> should_min_fill;
                SetMyTurn(true, should_min_fill);
              } else if (message == "now_turn") {
                int now_turn;
                packet >> now_turn;
                SetTurnsIndex(now_turn);
              }
              if (message == "winner") {
                string winner;
                int winner_score;
                packet >> winner >> winner_score;
                cout << "winner: " << winner << endl;
                SetWinner(winner, winner_score);
              }
              if (message == "lose") {
                int lose_score;
                packet >> lose_score;
                cout << "lose: " << lose_score << endl;
                SetLose(lose_score);
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
                  AddPlayersIds(ID);
                  cout << "new ID " << ID << endl;
                  //   IDs.push_back(ID);
                }
                AddPlayersBut();
              }
              if (message == "over_round") {
                cout << "over_round" << endl;
                OverRoundFn();
              }
            }
          }
        });
    receive_message.detach();
  }

  void SendPacketToServer(Packet packet) {
    if (socket->send(packet) == Socket::Done) {
      cout << "send packet to server successfully" << endl;
    } else {
      cout << "send packet to server failed" << endl;
    }
  }

  void setID(string IDset) { ID = IDset; }

  void SetAlertInterface(function<void(string)> AlertInterface) {
    Alert = AlertInterface;
  }
};

#endif