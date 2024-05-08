#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include <thread>
#include <functional>

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

    int turn = -1, round = 0;

    bool over_turn = false;
    int player_chips_value[4] = { 0 };

    int turns_index[MAX_PLAYERS] = { 0 };
    bool player_give_up[MAX_PLAYERS] = { false };
    int player_cards[MAX_PLAYERS][2];
    int normal_chips_value = 0;

    vector<int> public_card;
    int round_open_card_number[3] = { 3, 1, 1 };

    // 计算一手牌的价值
    int CalculateHandValue(const vector<int>& hand) {
        // 排序手中的牌，使得牌面从小到大排列
        vector<int> sorted_hand = hand;
        sort(sorted_hand.begin(), sorted_hand.end());

        // 计算各种牌型的价值
        int hand_value = 0;

        // 判断是否是同花
        bool is_flush = true;
        for (int i = 1; i < sorted_hand.size(); ++i) {
            if (sorted_hand[i] / 13 != sorted_hand[0] / 13) {
                is_flush = false;
                break;
            }
        }

        // 判断是否是顺子
        bool is_straight = true;
        for (int i = 1; i < sorted_hand.size(); ++i) {
            if (sorted_hand[i] % 13 != (sorted_hand[i - 1] % 13) + 1) {
                is_straight = false;
                break;
            }
        }

        // 计算各种牌型的价值
        if (is_flush && is_straight) {
            // 同花顺
            hand_value = STRAIGHT_FLUSH;
        }
        else if (is_straight) {
            // 顺子
            hand_value = STRAIGHT;
        }
        else if (is_flush) {
            // 同花
            hand_value = FLUSH;
        }
        else {
            // 计算其他牌型
            vector<int> card_count(13, 0); // 统计每种牌面的数量
            for (int card : sorted_hand) {
                card_count[card % 13]++;
            }

            // 计算相同牌面的数量
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
                // 一对和三条组合
                hand_value = FULL_HOUSE;
            }
            else if (pair_count == 1) {
                // 一对
                hand_value = ONE_PAIR;
            }
            else if (pair_count == 2) {
                // 两对
                hand_value = TWO_PAIR;
            }
            else if (three_of_a_kind_value != -1) {
                // 三条
                hand_value = THREE_OF_A_KIND;
            }
            else if (four_of_a_kind_value != -1) {
                // 四条
                hand_value = FOUR_OF_A_KIND;
            }
            else {
                // 高牌
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
        /*thread delete_disconnected_clients([&] {
			while (!*GameStart) {
				deleteDisconnectedClients();
			}
			});
        delete_disconnected_clients.detach();*/
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

    void ReceiveMessage() {
        std::thread receive_message([&] {
			while (!*GameStart) {
				for (TcpSocket* client : *clients) {
					Packet packet;
					if (client->receive(packet) == Socket::Done) {
						string message;
						packet >> message;
						cout << "receive message from " << client->getRemoteAddress() << ": " << message << endl;
                        if (message == "over_turn" || message == "call") {
                            over_turn = true;
                            if (message == "call") {
                                player_chips_value[turns_index[turn]] = normal_chips_value;
                            }
                        }
                        else if (message == "fill") {
                            int chipValue;
                            packet >> chipValue;
                            cout << "receive chip value: " << chipValue << endl;
                            player_chips_value[turns_index[turn]] = +chipValue;
                            normal_chips_value += chipValue;
                        }
                        else if (message == "give_up") {
							player_give_up[turns_index[turn]] = true;
						}
					}
				}
			}
			});
		receive_message.detach();
    }

    int RandomSelectCard() {
        int selectCard = rand() % 52;
        int suit = selectCard / 13;
        int number = selectCard % 13;
        if (card[suit][number] == 0) {
			card[suit][number] = 1;
			return selectCard;
		}
		else {
			return RandomSelectCard();
		}
    }


    void GenerateTurnsIndex() {
        for (int i = 0; i < clients->size(); i++) {
            turns_index[i] = i;
        }
        turns_index[clients->size()] = clients->size();
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

    void RunTurns(function<void(int)> AddNewPublicCardFn,
        function<void(bool)> SetMyTurn) {
        while (round < 3) {
            int new_card = RandomSelectCard();
            public_card.push_back(new_card);
            Packet packet;
            packet << "public_card " << new_card;
            SendToAllClients(packet);

            for (int i = 0; i < clients->size() + 1; i++) {
                if (!player_give_up[turns_index[i]]) {
                    turn = turns_index[i];
                    if (turn == clients->size()) {
                        SetMyTurn(true);
                        continue;
                    }
                    over_turn = false;
                    while (!over_turn) {
                        // wait for over
                    }
                }
			}
            round++;
        }
        int winner = -1;
        int scores[4];
        int winner_score = 0;
        for (int i = 0; i < clients->size() + 1; i++) {
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
        for (int i = 0; i < clients->size() + 1; i++) {
            if (turns_index[i] != winner) {
				packet << "lose " << player_chips_value[turns_index[i]];
                clients->at(turns_index[i])->send(packet);
			}
        }
    }
};
