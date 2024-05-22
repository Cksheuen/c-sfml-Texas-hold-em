#ifndef USE_ALERT
#define USE_ALERT

#include <SFML/Graphics.hpp>
#include <vector>

#define ANIMATION_TIME 1.

using namespace std;
using namespace sf;

class UseAlert {
private:
  RenderWindow &window;
  vector<Text> alertList;
  vector<float> alertTime;
  Clock clock;
  int width, height;

  Color color = Color::Red;

public:
  Font font;
  UseAlert(RenderWindow &windowSet, int widthSet, int heightSet, Font fontSet);

  void addAlert(string alert);

  void printAlert();
};

#endif