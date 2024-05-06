#include <iostream>
#include <math.h>
#include <cassert>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>
#include "UseCard.h"
#include "UseButton.h"
#include "UseChip.h"
#include "UseServer.h"
#include "UseClient.h"
#include "UseUI.h"

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

bool StartInterface = true;
bool ChooseRoomInterface = false;
bool RoomOwnerInterface = false;
bool init = false;

template<class T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    assert(!(hi < lo));
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

float smoothstep(float edge0, float edge1, float x) {
    x = clamp(static_cast<double>((x - edge0) / (edge1 - edge0)), 0.0, 1.0);
    return x * x * (3 - 2 * x);
}

float step(float edge, float x) {
    return x < edge ? 0.0 : 1.0;
}

int main() {
    TcpListener listener;

    UseUI ui(WINDOW_WIDTH, WINDOW_HEIGHT, &listener);

    ui.RunProgressBar();
    ui.InitCards();
    ui.InitChips();

    ServerOrClient ModeChoose = ui.GameMenu();

    if (ModeChoose == ServerOrClient::Server) {
        std::cout << "start room owner interface" << endl;
        static bool GameStart = false;
        static int player_count = 0;
        vector<TcpSocket*> clients;
        UseServer server(&clients, &GameStart, &listener, [](vector<TcpSocket*>* clients) {
            player_count = clients->size();
            std::cout << "player count: " << player_count << endl;
            });

        server.WaitForConnection();

        ui.RoomOwnerInterface(server, &player_count, &GameStart);
    } else if (ModeChoose == ServerOrClient::Client) {
        ui.RunProgressBar();

        vector<int> room_list;

        bool search_room_complete = false;

        UseClient client(&room_list, &ui.bg_shader, &search_room_complete);
        client.SearchServerList();

        int room_index = ui.ChooseRoomInterface(client, &search_room_complete, room_list);
         ui.ClientGameInterface(room_index);
    }
}