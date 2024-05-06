class UseClient {
private:
    Shader* bg_shader;
    vector<int>* room_list;
    bool* search_room_complete;
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
};