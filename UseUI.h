#ifndef USE_UI
#define USE_UI

class UseServer;
class UseClient;
class UseAlert;
class UseChip;
class UseCard;
class UseButton;

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace sf;

#define BUTTON_HEIGHT WINDOW_HEIGHT / 5

#define INPUT_ID_HELP 0
#define GAME_MENU_HELP 1
#define ROOM_OWNER_HELP 2
#define CHOOSE_ROOM_HELP 3
#define GAME_HELP 4
#define RESTART_HELP 5

#define INPUT_ID_HELP_CONTENT                                                  \
  "Please input your ID in the box below\n, and click the button to confirm."
#define GAME_MENU_HELP_CONTENT "Please choose to be a server or a client."
#define ROOM_OWNER_HELP_CONTENT                                                \
  "Please wait for other players to join the room."
#define CHOOSE_ROOM_HELP_CONTENT                                               \
  "Please choose a room(based on the port) to join."
#define GAME_HELP_CONTENT "Please wait for your turn."
#define RESTART_HELP_CONTENT                                                   \
  "Please wait for other players to restart the game,\nthe box rounded by "    \
  "line is ready to restart."

enum ServerOrClient { Server, Client };

class UseUI {
private:
  TcpListener *listener;
  RenderWindow window;
  Clock clock;
  RenderStates bg_states;
  RectangleShape bg_shape;

  Font tvt_font, normal_font;

  UseCard *cards[52];
  UseChip *chips[8];

  UseAlert *alert;

  UseButton *help_button[6], *call_for_help_button;
  const string help_button_text[6] = {
      INPUT_ID_HELP_CONTENT,    GAME_MENU_HELP_CONTENT, ROOM_OWNER_HELP_CONTENT,
      CHOOSE_ROOM_HELP_CONTENT, GAME_HELP_CONTENT,      RESTART_HELP_CONTENT};

  bool help_show = false;

  vector<float> move_card_start_time, move_card_end_angle, move_card_time;
  vector<Vector2i> move_card_dest;

  bool my_turn = false;

  mutex my_turn_mtx;

  int init_complete = 0;
  bool to_fill = false, to_dec = false;
  TcpSocket *socket = new TcpSocket;
  int join_room_index = -1;

  vector<UseButton *> PlayersButs;

  int turn_ID, should_min_fill = 0, now_fill = 0;

  Text should_fill_text, now_fill_text, winner_text, score_text;

  bool game_over = false;

  vector<string> IDs;

  float points;

  bool card_status_change = false;

  string own_ID;

  int port = 3000;

  bool eio = false;

public:
  int WINDOW_WIDTH, WINDOW_HEIGHT;
  vector<int> show_cards;
  Shader bg_shader;

  UseUI(int widthSet, int heightSet, TcpListener *listenerSet);
  void UpdateProgressBar(float percent);
  void BasicUI(std::function<void()> InsertFn);
  void BasicCoverUI(std::function<void()> InsertFn);
  void RunProgressBar();
  void InitCards();
  void InitChips();

  ServerOrClient GameMenu();

  void HelpButtonRun(int index);

  void RoomOwnerInterface(UseServer &server, int *player_count,
                          bool *GameStart);
  int ChooseRoomInterface(UseClient client, bool *search_room_complete,
                          vector<int> &room_list);

  bool GameInterface(function<void()> server_init,
                     function<void(Packet)> send_method);

  void AddMoveCard(int cardIndex, float x, float y, float angle);

  void AddNewPublicCard(int cardIndex);

  void SetMyTurn(bool my_turn_set, int should_min_fill_set);

  void InputContent(string *content);

  void AlertInterface(string alertCon);

  void AddPlayersId(string ID);

  void AddPlayersBut();
  void setTurnsIndex(int turnsIndexSet);

  void OverRound();

  void SetWinner(string winner, int win_score);

  void SetLose(int lose_score);

  void SetEio(bool eioSet);

  void SetReadyToRestart(int index);

  void SetGameStart();

  void SetPlayerExit(int index);
};

#endif