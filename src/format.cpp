#include <string>

#include "format.h"

using std::string;

string beautify(int num){
  if (num < 10 ){
    return "0" + std::to_string(num);
  } else {
    return std::to_string(num);
  }
}


// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
  const int ONE_HOUR = 60 * 60;
  const int ONE_MINUE = 60;
  long secs_left = seconds;

  const int hours = secs_left / ONE_HOUR;
  secs_left -= (hours * ONE_HOUR);
  const int minutes = secs_left / ONE_MINUE;
  secs_left -= (minutes * ONE_MINUE);
  const int secs = secs_left;
  
  return (beautify(hours) + ":" + beautify(minutes) + ":" + beautify(secs));
}