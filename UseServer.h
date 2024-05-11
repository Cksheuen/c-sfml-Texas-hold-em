#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include <thread>
#include <functional>
#include <random>
#include <mutex>

using namespace std;
using namespace sf;

#define MAX_PLAYERS 10

enum HandType {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
};

class UseServer {
private:
    vector<TcpSocket*>* clients;
    bool* GameStart;
    TcpListener* listener;
    std::function<void(vector<TcpSocket*>*)> callback;

    bool card[4][13] = { 0 };
    int OwnCard[2] = { -1 };

    int turn = -1, round = -1;

    bool over_turn = false;
    int player_chips_value[4] = { 0 };

    vector<int> turns_index;
    bool player_give_up[MAX_PLAYERS] = { false };
    int player_cards[MAX_PLAYERS][2];
    int normal_chips_value = 0;

    vector<int> public_card;
    int round_open_card_number[3] = { 3, 1, 1 };

    mutex mtx;

    int CalculateHandValue(const vector<int>& hand) {
        vector<int> sorted_hand = hand;
        sort(sorted_hand.begin(), sorted_hand.end());

        int hand_value = 0;

        bool is_flush = true;
        for (int i = 1; i < sorted_hand.size(); ++i) {
            if (sorted_hand[i] / 13 != sorted_hand[0] / 13) {
                is_flush = false;
                break;
            }
        }

        bool is_straight = true;
        for (int i = 1; i < sorted_hand.size(); ++i) {
            if (sorted_hand[i] % 13 != (sorted_hand[i - 1] % 13) + 1) {
                is_straight = false;
                break;
            }
        }

        if (is_flush && is_straight) {
            hand_value = STRAIGHT_FLUSH;
        }
        else if (is_straight) {
            hand_value = STRAIGHT;
        }
        else if (is_flush) {
            hand_value = FLUSH;
        }
        else {
            vector<int> card_count(13, 0);
            for (int card : sorted_hand) {
                card_count[card % 13]++;
            }

            int pair_count = 0;
            int three_of_a_kind_value = -1;
            int four_of_a_kind_value = -1;
            for (int count : card_count) {
                if (count == 2) {
                    pair_count++;
                }
                else if (count == 3) {
                    three_of_a_kind_value = distance(card_count.begin(), find(card_count.begin(), card_count.end(), 3));
                }
                else if (count == 4) {
                    four_of_a_kind_value = distance(card_count.begin(), find(card_count.begin(), card_count.end(), 4));
                }
            }

            if (pair_count == 1 && three_of_a_kind_value != -1) {
                hand_value = FULL_HOUSE;
            }
            else if (pair_count == 1) {
                hand_value = ONE_PAIR;
            }
            else if (pair_count == 2) {
                hand_value = TWO_PAIR;
            }
            else if (three_of_a_kind_value != -1) {
                hand_value = THREE_OF_A_KIND;
            }
            else if (four_of_a_kind_value != -1) {
                hand_value = FOUR_OF_A_KIND;
            }
            else {
                hand_value = HIGH_CARD;
            }
        }

        return hand_value;
    }
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
        thread wait_for_clients([&] {
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

    void ReceiveMessageFromAsync(int index) {
        if (index > clients->size() || index < 0) {
            return;
        }
        cout << "wait for message from player " << index << endl;
        TcpSocket* client = clients->at(index);
        Packet sendPacket;
        sendPacket << "I'm wating for your answer";
        client->send(sendPacket);

        Packet responsePacket;
        if (client->receive(responsePacket) == Socket::Done) {
            string responseMessage;
            responsePacket >> responseMessage;
            cout << "Received response from " << client->getRemoteAddress() << ": " << responseMessage << endl;

            while (true) {
                Packet packet;
                if (client->receive(packet) == Socket::Done) {
                    string message;
                    packet >> message;
                    cout << "receive message from " << client->getRemoteAddress() << ": " << message << endl;
                    if (message == "over_turn" || message == "call" || message == "give_up") {
                        lock_guard<mutex> lock(mtx);
                        over_turn = true;
                        if (message == "call") {
                            player_chips_value[index] = normal_chips_value;
                        }
                        else if (message == "give_up") {
                            player_give_up[index] = true;
                        }
                        break;
                    }
                    else if (message == "fill") {
                        int chipValue;
                        packet >> chipValue;
                        cout << "receive chip value: " << chipValue << endl;
                        player_chips_value[index] = +chipValue;
                        normal_chips_value += chipValue;
                    }
                }
            }
            
        }
        
        
    }

    void ReceiveOwnMessage(Packet packet) {
        string message;
        packet >> message;
        cout << "receive message from self: " << message << endl;
        if (message == "over_turn" || message == "call" || message == "give_up") {
            lock_guard<mutex> lock(mtx);
            over_turn = true;
            cout << "turn the over_turn to true self" << endl;
            if (message == "call") {
                player_chips_value[turn] = normal_chips_value;
            }
            else if (message == "give_up") {
                player_give_up[turn] = true;
            }
        }
        else if (message == "fill") {
            int chipValue;
            packet >> chipValue;
            cout << "receive chip value: " << chipValue << endl;
            player_chips_value[turn] = +chipValue;
            normal_chips_value += chipValue;
        }
        else if (message == "give_up") {
            player_give_up[turn] = true;
        }
    }

    int RandomSelectCard() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distrib(0, 51);

        while (true) {
            int selectCard = distrib(gen);
            int suit = selectCard / 13;
            int number = selectCard % 13;
            if (card[suit][number] == 0) {
                card[suit][number] = 1;
                return selectCard;
            }
        }
    }


    void GenerateTurnsIndex() {
        for (int i = 0; i < clients->size(); i++) {
            turns_index.push_back(i);
        }
        
        turns_index.push_back(clients->size());
        for (int i = 0; i < clients->size(); i++) {
            int randomIndex = rand() % (clients->size() + 1);
            int temp = turns_index[i];
            turns_index[i] = turns_index[randomIndex];
            turns_index[randomIndex] = temp;
        }
    }

    void SendCardToAll(function<void(int, int)> MoveCardFn) {
        cout << "generate index" << endl;
        GenerateTurnsIndex();
        for (int i = 0; i < clients->size() + 1; i++) {
            cout << turns_index[i] << " ";
        }
        cout << endl;
        cout << "generate card" << endl;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < clients->size() + 1; j++) {
				player_cards[turns_index[j]][i] = RandomSelectCard();
			}
        }
        cout << "send card" << endl;
        for (int i = 0; i < clients->size() + 1; i++) {
            cout << "player " << turns_index[i] << " has card " 
                << player_cards[turns_index[i]][0] << player_cards[turns_index[i]][1] << endl;
           
            if (turns_index[i] == clients->size()) { 
                cout << "send card to server self" << endl;
                cout << player_cards[turns_index[i]][0] << player_cards[turns_index[i]][1] << endl;
                MoveCardFn(player_cards[turns_index[i]][0], player_cards[turns_index[i]][1]);
				continue;
			}
            Packet packet;
            packet << "send_card" << player_cards[turns_index[i]][0] << player_cards[turns_index[i]][1];

			clients->at(turns_index[i])->send(packet);
		}
	}

    void SendToAllClients(Packet packet) {
        for (TcpSocket* client : *clients) {
			client->send(packet);
		}
	}

    void SendNewPublicCard() {
        int new_card = RandomSelectCard();
        cout << "new public card: " << new_card << endl;
        public_card.push_back(new_card);
        cout << "public card size: " << public_card.size() << endl;
        Packet packet;
        packet << "public_card" << new_card;
        SendToAllClients(packet);
    }

    void RunTurns(function<void(int)> AddNewPublicCardFn,
        function<void(bool)> SetMyTurn) {
        thread run_turns_thread([&, AddNewPublicCardFn = move(AddNewPublicCardFn),
            SetMyTurn = move(SetMyTurn)] {
            cout << "we have " << clients->size() + 1 << " players" << endl;
            cout << " they are ";
            for (int i = 0; i < clients->size() + 1; i++) {
				cout << turns_index[i] << " ";
			}
            cout << endl;
            while (round < 3) {
                cout << "round " << round << endl;

                if (round != -1)
                    for (int i = 0; i < round_open_card_number[round]; i++) {
					    SendNewPublicCard();
                        cout << "end send new public card" << endl;
                        AddNewPublicCardFn(public_card.back());
				    }


                for (int i = 0; i < turns_index.size(); i++) {
                    if (!player_give_up[turns_index[i]]) {
                        turn = turns_index[i];
                        if (turn == clients->size()) {
                            SetMyTurn(true);
                            over_turn = false;
                            cout << "wait for over from player " << turn << endl;

                            unique_lock<mutex> lock(mtx);
                            while (!over_turn) {
                                lock.unlock();
                                this_thread::sleep_for(chrono::milliseconds(100));
                                lock.lock();
                            }
                            cout << "player " << turn << " is over" << endl;
                        }
                        else {
                            
                            Packet packet;
                            packet << "your_turn";
                            clients->at(turn)->send(packet);
                            ReceiveMessageFromAsync(turn);
                            cout << "received over" << endl;
                        }
                    }
                }
                round++;
            }
            int winner = -1;
            int scores[4];
            int winner_score = 0;
            for (int i = 0; i < turns_index.size(); i++) {
                winner_score += player_chips_value[turns_index[i]];
                if (!player_give_up[turns_index[i]]) {
                    vector<int> hand;
                    for (int j = 0; j < 2; j++) {
                        hand.push_back(player_cards[turns_index[i]][j]);
                    }
                    for (int j = 0; j < public_card.size(); j++) {
                        hand.push_back(public_card[j]);
                    }
                    scores[turns_index[i]] = CalculateHandValue(hand);
                    if (winner == -1 || scores[turns_index[i]] > scores[winner]) {
                        winner = turns_index[i];
                    }
                }
            }
            winner_score -= player_chips_value[winner];
            cout << "winner is player " << winner << endl;
            Packet packet;
            packet << "winner " << winner;
            SendToAllClients(packet);
            for (int i = 0; i < turns_index.size(); i++) {
                if (turns_index[i] != winner) {
                    packet << "lose " << player_chips_value[turns_index[i]];
                    clients->at(turns_index[i])->send(packet);
                }
            }
			});
        run_turns_thread.detach();
    }
};
