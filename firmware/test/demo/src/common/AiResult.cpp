#include "AiResult.h"
#include <ctype.h>

static String valueForKey(const String& payload, char key) {
  int pos = payload.indexOf(String(key) + ":");
  if (pos < 0) {
    pos = payload.indexOf(String(key) + "=");
  }
  if (pos < 0) {
    char lower = char(tolower(key));
    pos = payload.indexOf(String(lower) + ":");
    if (pos < 0) {
      pos = payload.indexOf(String(lower) + "=");
    }
  }
  if (pos < 0) {
    return "";
  }
  int start = pos + 2;
  int end = payload.indexOf(',', start);
  if (end < 0) {
    end = payload.indexOf(';', start);
  }
  if (end < 0) {
    end = payload.indexOf('\r', start);
  }
  if (end < 0) {
    end = payload.indexOf('\n', start);
  }
  if (end < 0) {
    end = payload.length();
  }
  String value = payload.substring(start, end);
  value.trim();
  return value;
}

bool ParseAiPayload(const String& payload, AiResult& out) {
  out.grade = valueForKey(payload, 'G');
  out.meat = valueForKey(payload, 'M');
  out.type = valueForKey(payload, 'C');
  if (out.type.length() == 0) {
    out.type = valueForKey(payload, 'T');
  }
  String weight = valueForKey(payload, 'W');
  out.weight = weight.length() > 0 ? weight.toFloat() : 0.0f;
  out.valid = out.grade.length() > 0 && out.meat.length() > 0 && out.type.length() > 0;
  return out.valid;
}

String AiResult_Line1(const AiResult& result) {
  return String("W:") + String(result.weight, 0) + " G:" + result.grade;
}

String AiResult_Line2(const AiResult& result) {
  return String("M:") + result.meat + " C:" + result.type;
}
