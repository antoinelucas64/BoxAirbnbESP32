#include "StringUtils.h"
#include <cstring>
#include <cctype>

void aa::upperCase(char * line){
  for (int i = 0 ; i < strlen(line) && i < 250; i++)
  {
    line[i] = std::toupper(line[i]);
  }
}

bool aa::contains(const char * line, const char * word){
  char lineUpper [250];
  char wordUpper [250];
  strncpy(lineUpper, line, 250);
  strncpy(wordUpper, word, 250);
  upperCase(lineUpper);
  upperCase(wordUpper);
  return strstr(lineUpper,wordUpper) != NULL; 
}

String aa::secondWord(const String & line){
  String out;
  bool skipFirst = false;
  bool findSep = true;
  for (int i = 0 ; i < line.length(); i++){
    if(line[i] == ' '){
      if(findSep ) continue;
      else {
         findSep = true;
         if(skipFirst) break;
         skipFirst = true;
      }
    }
    else {
      findSep = false;
    }
    if(skipFirst && line[i] != ' ') out += line[i];
  }
  return out;
}

bool aa::sameWords(const char * first , const char * second){
  return strstr(first,second) == first && strstr(second,first) == second;
}
