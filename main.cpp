#include "UseCal.h"
#include "UseClient.h"
#include "UseServer.h"
#include "UseUI.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cassert>
#include <iostream>
#include <math.h>
#include <thread>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 400;

bool StartInterface = true;
bool ChooseRoomInterface = false;
bool init = false;

int main() {
  TcpListener listener;

  UseUI ui(WINDOW_WIDTH, WINDOW_HEIGHT, &listener);

  string ID;

  ui.RunProgressBar();
  ui.InitCards();
  ui.InitChips();

  ui.InputContent(&ID);

  cout << "ID is " << ID << endl;

  ServerOrClient ModeChoose = ui.GameMenu();

  if (ModeChoose == ServerOrClient::Server) {
    static bool GameStart = false;
    static int player_count = 0;
    vector<TcpSocket *> clients;
    unique_ptr<UseServer> server = make_unique<UseServer>(
        &clients, &GameStart, &listener,
        [](vector<TcpSocket *> *clients) { player_count = clients->size(); });
    server->setID(ID);

    server->WaitForConnection();

    ui.RoomOwnerInterface(*server, &player_count, &GameStart);

    cout << "Game Start" << endl;

    server->SetUIInterface(&ui);

    bool keep_game = true;

    while (keep_game) {
      keep_game = ui.GameInterface(
          [&] {
            server->SendCardToAll([&ui](int x, int y) {
              cout << "Send Card To Server Self" << endl;
              ui.AddMoveCard(x, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 * 2, 0);
              ui.AddMoveCard(y, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 3 * 2, 0);
            });

            server->RunTurns();
          },
          [&](Packet packet) {
            // server->ReceiveOwnMessage(packet);
            server->DealWithMessage(packet);
          });
    }

  } else if (ModeChoose == ServerOrClient::Client) {
    ui.RunProgressBar();

    vector<int> room_list;

    bool search_room_complete = false;

    UseClient client(&room_list, &ui.bg_shader, &search_room_complete);
    client.setID(ID);
    client.SearchServerList();

    client.SetUI(&ui);

    int room_index =
        ui.ChooseRoomInterface(client, &search_room_complete, room_list);
    client.ChooseRoom(room_index);
    client.SetAlertInterface([&ui](string alert) { ui.AlertInterface(alert); });
    client.ReceiveMessage();

    ui.GameInterface([] {},
                     [&](Packet packet) {
                       cout << "client send packet to server" << endl;
                       client.SendPacketToServer(packet);
                     });

    while (1)
      ;
  }
}