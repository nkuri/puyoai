#ifndef CORE_STATE_H_
#define CORE_STATE_H_

#include <string>

enum State {
  STATE_NONE = 0,
  STATE_YOU_CAN_PLAY = 1 << 0,
  STATE_WNEXT_APPEARED = 1 << 2,
  STATE_YOU_GROUNDED = 1 << 4,
  /* STATE_YOU_WIN = 1 << 6, */
  STATE_CHAIN_DONE = 1 << 8,
  STATE_OJAMA_DROPPED = 1 << 10,
};

inline std::string GetStateString(int st) {
  std::string r;
  r.resize(5);
  r[0] = ((st & STATE_YOU_CAN_PLAY) != 0) ? 'P' : '-';
  r[1] = ((st & STATE_WNEXT_APPEARED) != 0) ? 'W' : '-';
  r[2] = ((st & STATE_YOU_GROUNDED) != 0) ? 'G' : '-';
  r[3] = ((st & STATE_CHAIN_DONE) != 0) ? 'C' : '-';
  r[4] = ((st & STATE_OJAMA_DROPPED) != 0) ? 'O' : '-';
  return r;
}

#endif  // CORE_STATE_H_
