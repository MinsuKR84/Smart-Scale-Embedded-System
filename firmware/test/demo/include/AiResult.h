#ifndef AI_RESULT_H
#define AI_RESULT_H

#include <Arduino.h>

struct AiResult {
  String grade;
  String meat;
  String type;
  float weight;
  bool valid;
};

bool ParseAiPayload(const String& payload, AiResult& out);
String AiResult_Line1(const AiResult& result);
String AiResult_Line2(const AiResult& result);

#endif
