#ifndef USE_CLIENT
#define USE_CLIENT

#include <functional>
#include <thread>
#include <SFML/Network.hpp>

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
              packet << Socket::Disconnected;
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
                      function<void(bool)> SetMyTurn,
                      function<void(string)> AddPlayersBut,
                      function<void(string)> SetTurnsIndex) {
    cout << "start receive message" << endl;
    socket = new TcpSocket;
    if (socket->connect("localhost", room_index)) {
      cout << "connect success" << endl;
    } else {
      cout << "connect fail" << endl;
    }
    sendID();

    thread receive_message([&, MoveCardFn = move(MoveCardFn),
                            AddNewPublicCardFn = move(AddNewPublicCardFn),
                            SetMyTurn = move(SetMyTurn),
                            AddPlayersBut = move(AddPlayersBut),
                            SetTurnsIndex = move(SetTurnsIndex), this]() {
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
            SetMyTurn(true);
          } else if (message == "turns_index") {
            int turns_index;
            packet >> turns_index;
            cout << "turns_index: " << turns_index << endl;
          }
          if (message == "winner") {
            int winner;
            packet >> winner;
            cout << "winner: " << winner << endl;
            Alert("You win");
          }
          if (message == "lose") {
            int lose_score;
            packet >> lose_score;
            cout << "lose: " << lose_score << endl;
            Alert("You lose");
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
              SetTurnsIndex(ID);
              //   IDs.push_back(ID);
            }
          }
        }
      }
    });
    receive_message.detach();
  }

  void SendPacketToServer(Packet packet) { socket->send(packet); }

  void setID(string IDset) { ID = IDset; }

  void SetAlertInterface(function<void(string)> AlertInterface) {
    Alert = AlertInterface;
  }
};

#endif