#ifndef CORE_PUYO_H_
#define CORE_PUYO_H_

enum PuyoColor {
  EMPTY = 0,
  OJAMA = 1,
  WALL = 2,
  RED = 4,
  BLUE = 5,
  YELLOW = 6,
  GREEN = 7,
};

typedef unsigned char Puyo;

// For backward compatibility
typedef PuyoColor Colors;

#endif