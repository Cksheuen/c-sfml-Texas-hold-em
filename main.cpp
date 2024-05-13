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

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 400;

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
        vector<TcpSocket*> clients;
        unique_ptr<UseServer> server = make_unique<UseServer>(&clients, &GameStart, &listener, [](vector<TcpSocket*>* clients) {
            player_count = clients->size();
            });
        server->setID(ID);

        server->WaitForConnection();
        ui.RoomOwnerInterface(*server, &player_count, &GameStart);

        cout << "Game Start" << endl;
        
        ui.GameInterface([&] {
            server->SendCardToAll([&ui](int x, int y) {
                cout << "Send Card To Server Self" << endl;
                ui.AddMoveCard(x, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 * 2, 0);
                ui.AddMoveCard(y, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 3 * 2, 0);
            });

            server->RunTurns([&ui](int new_public_card) {
                cout << "to run AddNewPublicCard" << endl;
                ui.AddNewPublicCard(new_public_card);
                cout << "end AddNewPublicCard" << endl;
                },
                [&ui](bool my_turn) {
                    ui.SetMyTurn(my_turn);
            });
        },
        [&] (Packet packet) {
            server->ReceiveOwnMessage(packet);
        });
    } else if (ModeChoose == ServerOrClient::Client) {
        ui.RunProgressBar();

        vector<int> room_list;

        bool search_room_complete = false;

        UseClient client(&room_list, &ui.bg_shader, &search_room_complete);
        client.setID(ID);
        client.SearchServerList();

        int room_index = ui.ChooseRoomInterface(client, &search_room_complete, room_list);
        client.ChooseRoom(room_index);
        client.ReceiveMessage([&ui](int x, int y) {
            ui.AddMoveCard(x, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 * 2, 0);
            ui.AddMoveCard(y, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 3 * 2, 0);
            },
            [&ui](int new_public_card) {
                cout << "new public card: " << new_public_card << endl;
                ui.AddNewPublicCard(new_public_card);
            },
            [&ui](bool my_turn) {
                ui.SetMyTurn(my_turn);
            });
            
        ui.GameInterface([]{},
            [&] (Packet packet) {
                client.SendPacketToServer(packet);
            }
        );
        
        while (1);
    }
}