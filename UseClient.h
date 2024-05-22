#ifndef USE_CLIENT
#define USE_CLIENT

class UseUI;

#include "UseCal.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <functional>
#include <thread>
#include <vector>

using namespace std;
using namespace sf;

class UseClient {
private:
  UseUI *ui;
  Shader *bg_shader;
  vector<int> *room_list;
  bool *search_room_complete;
  int room_index;
  TcpSocket *socket;

  string ID;

  function<void(string)> Alert;

public:
  UseClient(vector<int> *room_list_set, Shader *bg_shader_set,
            bool *search_room_complete_set);
  void SetUI(UseUI *ui_set);
  void SearchServerList();

  void ChooseRoom(int room_index_set);

  void sendID();

  void ReceiveMessage();

  void SendPacketToServer(Packet packet);

  void setID(string IDset);

  void SetAlertInterface(function<void(string)> AlertInterface);
};

#endif