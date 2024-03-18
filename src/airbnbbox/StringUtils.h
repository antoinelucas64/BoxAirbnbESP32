#ifndef A_A_STRINGUTILS
#define A_A_STRINGUTILS 1
#include <WString.h>

namespace aa {

  void upperCase(char * line);
  /*
  *
  * Check if word is in line, independant to case.
  */
  bool contains(const char * line, const  char * word);
  String secondWord(const String & line);

  bool sameWords(const char * first , const char * second);

};
#endif