#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include <thread>
#include <functional>

using namespace std;
using namespace sf;

class UseServer {
private:
    vector<TcpSocket*>* clients;
    bool* GameStart;
    TcpListener* listener;
    std::function<void(vector<TcpSocket*>*)> callback;

public:
    UseServer(vector<TcpSocket*>* clientsSet, bool* GameStartSet, TcpListener* listenerSet, std::function<void(vector<TcpSocket*>*)> callbackSet)
        : clients(clientsSet), GameStart(GameStartSet), listener(listenerSet), callback(callbackSet) {};

    void deleteDisconnectedClients() {
        for (auto it = clients->begin(); it != clients->end();) {
            TcpSocket* client = *it;
            Packet packet;
            if (client->receive(packet) == Socket::Disconnected) {
                delete client;
                it = clients->erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void WaitForConnection() {
        std::thread wait_for_clients([&] {
            while (clients->size() < 4 && !*GameStart) {
                deleteDisconnectedClients();
                TcpSocket* client = new TcpSocket;
                if (listener->accept(*client) == Socket::Done) {
                    clients->push_back(client);
                    Packet packet;
                    packet << "welcome to the game room";
                    client->send(packet);
                    cout << "send welcome message to " << client->getRemoteAddress() << endl;
                    std::cout << "new connection from " << client->getRemoteAddress() << endl;
                    callback(clients);
                }
                else {
                    delete client;
                }
            }
            *GameStart = true;
            });
        wait_for_clients.detach();
    }
};
