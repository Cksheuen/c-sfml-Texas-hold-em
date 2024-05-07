#include <functional>

class UseClient {
private:
    Shader* bg_shader;
    vector<int>* room_list;
    bool* search_room_complete;
    int OwnCard[2] = { -1 };
    int room_index;
public:
    UseClient(vector<int>* room_list_set, Shader* bg_shader_set, bool* search_room_complete_set)
        : room_list(room_list_set), bg_shader(bg_shader_set), search_room_complete(search_room_complete_set) {};
    void SearchServerList() {
        thread search_room([&](vector<int>* room_list, Shader* bg_shader) {
            for (int port = 3000; port < 4000; port++) {
                sf::TcpSocket* socket = new TcpSocket;
                if (socket->connect("localhost", port) == sf::Socket::Done) {
                    room_list->push_back(port);
                    cout << "room " << port << " is available" << endl;
                    bg_shader->setUniform("percent", (float)((port - 3000.0) / 2000.0));

                    Packet packet;
                    packet << "disconnect";
                    socket->send(packet);
                    socket->disconnect();
                }
                else {
                    delete socket;
                    break;
                }
            }
            *search_room_complete = true;
            }, room_list, bg_shader);
        search_room.detach();
    }

    void ChooseRoom(int room_index_set) {
        room_index = room_index_set;
    }

    void ReceiveMessage(function<void(int, int)> MoveCardFn,
        function<void(int)> AddNewPublicCardFn,
        function<void(bool)> SetMyTurn) {
        	thread receive_message([=, &MoveCardFn, &AddNewPublicCardFn]() {
                sf::TcpSocket* socket = new TcpSocket;
			    while (true) {
					if (socket->connect("localhost", room_index) == sf::Socket::Done) {
						Packet packet;
						if (socket->receive(packet) == sf::Socket::Done) {
							string message;
							packet >> message;
							cout << message << endl;
                            if (message == "send_card") {
                                packet >> OwnCard[0] >> OwnCard[1];
                                cout << "receive card: " << OwnCard[0] << " " << OwnCard[1] << endl;
                                MoveCardFn(OwnCard[0], OwnCard[1]);
                            }
                            if (message == "public_card") {
								cout << "public_card" << endl;
                                int new_public_card;
                                packet >> new_public_card;
                                AddNewPublicCardFn(new_public_card);
							}
                            if (message == "your_turn") {
								cout << "your turn" << endl;
								SetMyTurn(true);
							}
                            if (message == "winner") {
                                int winner;
                                packet >> winner;
                                cout << "winner: " << winner << endl;
                            }
                            if (message == "lose") {
								int lose_score;
								packet >> lose_score;
								cout << "lose: " << lose_score << endl;
							}
                            
						}
					}
			    }
			});
		receive_message.detach();
    }
};