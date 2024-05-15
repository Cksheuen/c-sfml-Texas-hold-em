#ifndef USECAL_H
#define USECAL_H

#include <cassert>
#include <string>

using namespace std;

static int chips_value[8] = {1, 2, 5, 10, 20, 25, 50, 100};

template <class T> T clamp(T v, T lo, T hi) {
  assert(!(hi < lo));
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

static float smoothstep(float edge0, float edge1, float x) {
  x = clamp(static_cast<double>((x - edge0) / (edge1 - edge0)), 0.0, 1.0);
  return x * x * (3 - 2 * x);
}

static float step(float edge, float x) { return x < edge ? 0.0 : 1.0; }

static string cardTypeNumMap[5] = {"Heart", "Spade", "Club", "Diamond", "Joker"};

// 扑克牌类型
enum CardType {
  CardType_None = 0,
  CardType_Heart,   // 红桃
  CardType_Spade,   // 黑桃
  CardType_Club,    // 梅花
  CardType_Diamond, // 方块
  CardType_Joker,   // 大小王
};

static map<enum CardType, string> cardTypeMap = {{CardType_Heart, "Heart"},
                                          {CardType_Spade, "Spade"},
                                          {CardType_Club, "Club"},
                                          {CardType_Diamond, "Diamond"},
                                          {CardType_Joker, "Joker"}};

static string number_to_string(int number) {
  if (number == 1) {
    return "A";
  } else if (number > 1 && number <= 10) {
    return to_string(number);
  } else {
    switch (number) {
    case 11:
      return "J";
    case 12:
      return "Q";
    case 13:
      return "K";
    default:
      return "None";
    }
  }
}

static string TransformCardIndexToString(int cardIndex) {
  string cardType = cardTypeNumMap[cardIndex / 13];
  string cardNumber = number_to_string(cardIndex % 13 + 1);
  return cardType + " " + cardNumber;
}

#endif