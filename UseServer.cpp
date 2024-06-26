#include "UseServer.h"
#include "UseUI.h"
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

int UseServer::CalculateHandValue(const vector<int> &hand) {
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
  } else if (is_straight) {
    hand_value = STRAIGHT;
  } else if (is_flush) {
    hand_value = FLUSH;
  } else {
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
      } else if (count == 3) {
        three_of_a_kind_value = distance(
            card_count.begin(), find(card_count.begin(), card_count.end(), 3));
      } else if (count == 4) {
        four_of_a_kind_value = distance(
            card_count.begin(), find(card_count.begin(), card_count.end(), 4));
      }
    }

    if (pair_count == 1 && three_of_a_kind_value != -1) {
      hand_value = FULL_HOUSE;
    } else if (pair_count == 1) {
      hand_value = ONE_PAIR;
    } else if (pair_count == 2) {
      hand_value = TWO_PAIR;
    } else if (three_of_a_kind_value != -1) {
      hand_value = THREE_OF_A_KIND;
    } else if (four_of_a_kind_value != -1) {
      hand_value = FOUR_OF_A_KIND;
    } else {
      hand_value = HIGH_CARD;
    }
  }

  return hand_value;
}

UseServer::UseServer(vector<TcpSocket *> *clientsSet, bool *GameStartSet,
                     TcpListener *listenerSet,
                     std::function<void(vector<TcpSocket *> *)> callbackSet)
    : clients(clientsSet), GameStart(GameStartSet), listener(listenerSet),
      callback(callbackSet){};

void UseServer::deleteDisconnectedClients() {
  for (auto it = clients->begin(); it != clients->end();) {
    TcpSocket *client = *it;
    Packet packet;
    if (client->receive(packet) == Socket::Disconnected) {
      delete client;
      it = clients->erase(it);
    } else {
      ++it;
    }
  }
}

void UseServer::SetUIInterface(UseUI *uiSet) { ui = uiSet; }

void UseServer::WaitForConnection() {
  thread wait_for_clients([&] {
    while (clients->size() < MAX_PLAYERS && !*GameStart) {
      // deleteDisconnectedClients();
      TcpSocket *client = new TcpSocket;
      if (listener->accept(*client) == Socket::Done) {
        Packet packet;
        packet << "welcome to the game room";
        client->send(packet);
        packet.clear();
        cout << "send welcome message to " << client->getRemoteAddress()
             << endl;
        cout << "new connection from " << client->getRemoteAddress() << endl;
        clients->push_back(client);
        over_turn = true;
        ReceiveMessageFromAsync(clients->size() - 1);
        if (clients->size() < IDs.size() + 1) {
          // clients->push_back(client);
          ui->AlertInterface(IDs[clients->size() - 1] +
                             " has joined the game room");
          Packet packet;
          packet << "join_game_room" << IDs[clients->size() - 1];
          SendToAllClients(packet);
        } else {
          cout << "remove client" << endl;
          clients->pop_back();
        }
        callback(clients);
      } else {
        delete client;
      }
    }
    *GameStart = true;
  });
  wait_for_clients.detach();
}

string UseServer::ReceiveID(int index) {
  if (index > clients->size() || index < 0) {
    return "";
  }
  TcpSocket *client = clients->at(index);

  while (true) {
    Packet packet;
    if (client->receive(packet) == Socket::Done) {
      string message;
      packet >> message;
      cout << "ID from " << client->getRemoteAddress() << ": " << message
           << endl;
      return message;
    }
  }
}

void UseServer::DealWithMessage(Packet packet, int index) {
  if (index == -1)
    index = turn;
  cout << "deal with message" << endl;
  string message;
  packet >> message;
  cout << "receive message from " << index << ": " << message << endl;
  if (message == "over_turn" || message == "call" || message == "give_up") {
    lock_guard<mutex> lock(mtx);
    over_turn = true;
    if (message == "call") {
      player_chips_value[index] = now_round_base_fill + should_min_fill;
    } else if (message == "give_up") {
      player_give_up[index] = true;
    } else if (message == "over_turn") {
      should_min_fill = player_chips_value[index] - now_round_base_fill;
      cout << "player " << index << " over_turn, his/her all is "
           << player_chips_value[index] << endl;
    }
    cout << "turn the over_turn to true" << endl;
  } else if (message == "fill") {
    int chipValue;
    packet >> chipValue;
    cout << "receive chip value: " << chipValue << endl;

    normal_chips_value += chipValue;

    player_chips_value[index] += chipValue;
  } else if (message == "send_id") {
    cout << "send_id";
    string new_id = ReceiveID(index);
    cout << "this players' id is " << endl;
    cout << new_id << endl;
    IDs.push_back(new_id);
  } else if (message == "restart_game") {

    packet.clear();
    packet << "ready_to_restart" << index;

    player_over_prepare[index] = true;
    for (int i = 0; i < turns_index.size(); i++) {
      if (!player_over_prepare[turns_index[i]]) {
        return;
      }
    }
    eio = true;
    ui->SetEio(true);
    packet.clear();
    packet << "eio";
    SendToAllClients(packet);
  } else if (message == "exit_game") {
    player_over_prepare.erase(player_over_prepare.begin() + index);
    player_give_up.erase(player_give_up.begin() + index);
    player_chips_value.erase(player_chips_value.begin() + index);
    clients->erase(clients->begin() + index);
    IDs.erase(IDs.begin() + index);
    turns_index.erase(turns_index.begin() + index);
    for (int i = 0; i < turns_index.size(); i++) {
      if (turns_index[i] > index) {
        turns_index[i]--;
      }
    }
    Packet packet;
    packet << "exit_game" << index;
    SendToAllClients(packet);
  }
}

void UseServer::ReceiveMessageFromAsync(int index) {
  if (index > clients->size() || index < 0) {
    return;
  }
  cout << "wait for message from player " << index << endl;
  TcpSocket *client = clients->at(index);
  Packet sendPacket;
  sendPacket << "I'm wating for your answer";
  client->send(sendPacket);

  Packet responsePacket;
  if (client->receive(responsePacket) == Socket::Done) {
    string responseMessage;
    responsePacket >> responseMessage;
    cout << "Received response from " << client->getRemoteAddress() << ": "
         << responseMessage << endl;
    if (responseMessage == "just_searching")
      return;
    while (true) {
      Packet packet;
      if (client->receive(packet) == Socket::Done) {
        DealWithMessage(packet, index);
        cout << "end deal with message for a time, and now over_turn is "
             << over_turn << endl;
      }
      if (over_turn)
        return;
    }
  }
}

void UseServer::ReceiveOwnMessage(Packet packet) {
  string message;
  packet >> message;
  cout << "receive message from self: " << message << endl;

  if (message == "over_turn" || message == "call" || message == "give_up") {
    lock_guard<mutex> lock(mtx);
    over_turn = true;
    cout << "turn the over_turn to true self" << endl;
    if (message == "call") {
      player_chips_value[turn] = normal_chips_value;
    } else if (message == "give_up") {
      player_give_up[turn] = true;
    }
  } else if (message == "fill") {
    int chipValue;
    packet >> chipValue;
    cout << "receive chip value: " << chipValue << endl;
    player_chips_value[turn] = +chipValue;
    normal_chips_value += chipValue;
  } else if (message == "give_up") {
    player_give_up[turn] = true;
  }
}

int UseServer::RandomSelectCard() {
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

void UseServer::GenerateTurnsIndex() {
  turns_index.clear();

  for (int i = 0; i < clients->size(); i++) {
    turns_index.push_back(i);
  }

  turns_index.push_back(clients->size());
  IDs.push_back(ID);
  cout << "IDS" << endl;
  for (int i = 0; i < IDs.size(); i++) {
    cout << IDs[i] << " ";
  }
  cout << endl;
  for (int i = 0; i < clients->size(); i++) {
    int randomIndex = rand() % (clients->size() + 1);
    int temp = turns_index[i];
    turns_index[i] = turns_index[randomIndex];
    turns_index[randomIndex] = temp;
  }
}

void UseServer::SendTurnsIndex() {
  cout << "send turns index" << endl;
  Packet packet;
  packet << "turns_index";
  int len = turns_index.size();
  packet << len;
  for (int i = 0; i < len; i++) {
    packet << IDs[turns_index[i]];
    ui->AddPlayersId(IDs[turns_index[i]]);
  }
  ui->AddPlayersBut();
  SendToAllClients(packet);
}

void UseServer::SendCardToAll(function<void(int, int)> MoveCardFn) {
  cout << "generate index" << endl;
  GenerateTurnsIndex();
  for (int i = 0; i < clients->size() + 1; i++) {
    cout << turns_index[i] << " ";
  }

  SendTurnsIndex();

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
         << player_cards[turns_index[i]][0] << player_cards[turns_index[i]][1]
         << endl;

    if (turns_index[i] == clients->size()) {
      cout << "send card to server self" << endl;
      cout << player_cards[turns_index[i]][0] << player_cards[turns_index[i]][1]
           << endl;
      MoveCardFn(player_cards[turns_index[i]][0],
                 player_cards[turns_index[i]][1]);
      continue;
    }
    Packet packet;
    packet << "send_card" << player_cards[turns_index[i]][0]
           << player_cards[turns_index[i]][1];

    clients->at(turns_index[i])->send(packet);
  }
}

void UseServer::SendToAllClients(Packet packet) {
  for (TcpSocket *client : *clients) {
    client->send(packet);
  }
}

void UseServer::SendNewPublicCard() {
  int new_card = RandomSelectCard();
  cout << "new public card: " << new_card << endl;
  public_card.push_back(new_card);
  cout << "public card size: " << public_card.size() << endl;
  Packet packet;
  packet << "public_card" << new_card;
  SendToAllClients(packet);
}

bool UseServer::IsAllPlayersSame() {
  int same_count = -1, i = 0;
  if (should_min_fill == 0) {
    return false;
  }
  while (i < turns_index.size()) {
    cout << "i:  " << i << "player_chip: " << player_chips_value[i]
         << "same_count" << same_count << endl;
    while (player_give_up[i] && i < turns_index.size()) {
      i++;
    }
    if (i >= turns_index.size()) {
      return true;
    }
    if (same_count == -1) {
      same_count = player_chips_value[i];
    } else if (same_count != player_chips_value[i]) {
      return false;
    }
    i++;
  }
  now_round_base_fill = same_count;
  return true;
}

void UseServer::RunTurns() {
  thread run_turns_thread([&] {
    cout << "we have " << clients->size() + 1 << " players" << endl;
    cout << " they are ";
    for (int i = 0; i < clients->size() + 1; i++) {
      cout << IDs[turns_index[i]] << "(" << turns_index[i] << ") ";
    }
    cout << endl;
    bool first = false;
    player_chips_value.resize(clients->size() + 1);
    player_over_prepare.resize(clients->size() + 1);
    player_give_up.resize(clients->size() + 1);
    for (int i = 0; i < clients->size() + 1; i++) {
      player_chips_value[turns_index[i]] = 0;
      player_over_prepare[turns_index[i]] = false;
      player_give_up[turns_index[i]] = false;
    }
    while (round < 3) {
      cout << "round " << round << endl;

      should_min_fill = 0;

      if (round != -1)
        for (int i = 0; i < round_open_card_number[round]; i++) {
          SendNewPublicCard();
          cout << "end send new public card" << endl;
          ui->AddNewPublicCard(public_card.back());
        }
      first_fill = false;
      int i = 0;
      while (!IsAllPlayersSame()) {
        over_turn = false;
        if (!player_give_up[turns_index[i]]) {
          turn = turns_index[i];
          cout << "now turn is " << turn << endl;

          Packet packet;
          packet << "now_turn" << i;
          SendToAllClients(packet);
          ui->setTurnsIndex(i);

          if (turn == clients->size()) {
            if (round != -1 || i != 1)
              ui->SetMyTurn(true, should_min_fill);
            else {
              ui->SetMyTurn(false, should_min_fill * 2);
            }
            cout << "wait for over from player " << IDs[turn] << "(" << turn
                 << ")" << endl;

            unique_lock<mutex> lock(mtx);
            while (!over_turn) {
              lock.unlock();
              this_thread::sleep_for(chrono::milliseconds(100));
              lock.lock();
            }
            cout << "player " << IDs[turn] << "(" << turn << ") is over"
                 << endl;
          } else {
            Packet packet;
            packet << "your_turn" << should_min_fill;
            clients->at(turn)->send(packet);
            ReceiveMessageFromAsync(turn);
            cout << "received over" << endl;
          }
        }
        if (!first) {
          should_min_fill *= 2;
          first = true;
        }

        i = (i + 1) % turns_index.size();
      }
      Packet packet;
      packet << "over_round";
      SendToAllClients(packet);
      ui->OverRound();
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
    cout << "winner is player " << IDs[winner] << endl;
    Packet packet;
    packet << "winner" << IDs[winner] << winner_score;
    SendToAllClients(packet);
    ui->SetWinner(IDs[winner], winner_score);
    cout << "winner : " << winner << " turns_index.size() - 1 "
         << turns_index.size() - 1 << endl;
    for (int i = 0; i < turns_index.size(); i++) {
      if (turns_index[i] != winner) {
        if (turns_index[i] != turns_index.size() - 1) {
          cout << "send lose to " << IDs[turns_index[i]] << endl;
          packet.clear();
          packet << "lose" << player_chips_value[turns_index[i]];
          clients->at(turns_index[i])->send(packet);
        } else {
          cout << "send lose to self" << endl;
          ui->SetLose(player_chips_value[turns_index[i]]);
        }
      }
    }
  });
  run_turns_thread.detach();
}
void UseServer::setID(string IDset) { ID = IDset; }