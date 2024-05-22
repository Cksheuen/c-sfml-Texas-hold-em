#ifndef USE_SERVER
#define USE_SERVER

class UseUI;

#include <SFML/Network.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

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
  UseUI *ui;

  vector<TcpSocket *> *clients;
  bool *GameStart;
  TcpListener *listener;
  function<void(vector<TcpSocket *> *)> callback;

  bool small_blind = false, big_blind = false, first_fill = false;

  int should_min_fill = 0, now_round_base_fill = 0;

  bool card[4][13] = {0};
  int OwnCard[2] = {-1};

  int turn = -1, round = -1;

  bool over_turn = false;

  vector<int> turns_index, player_chips_value;
  vector<bool> player_give_up, player_over_prepare;
  /* bool player_give_up[MAX_PLAYERS] = {false},
       player_over_prepare[MAX_PLAYERS] = {false}; */
  int player_cards[MAX_PLAYERS][2];
  int normal_chips_value = 0;

  vector<int> public_card;
  int round_open_card_number[3] = {3, 1, 1};

  mutex mtx;

  string ID;
  vector<string> IDs;

  bool eio;

  int CalculateHandValue(const vector<int> &hand);

public:
  UseServer(vector<TcpSocket *> *clientsSet, bool *GameStartSet,
            TcpListener *listenerSet,
            std::function<void(vector<TcpSocket *> *)> callbackSet);

  void deleteDisconnectedClients();

  void SetUIInterface(UseUI *uiSet);

  void WaitForConnection();

  string ReceiveID(int index);

  void DealWithMessage(Packet packet, int index = -1);

  void ReceiveMessageFromAsync(int index);

  void ReceiveOwnMessage(Packet packet);

  int RandomSelectCard();

  void GenerateTurnsIndex();

  void SendTurnsIndex();

  void SendCardToAll(function<void(int, int)> MoveCardFn);

  void SendToAllClients(Packet packet);

  void SendNewPublicCard();

  bool IsAllPlayersSame();

  void RunTurns();

  void setID(string IDset);
};

#endif