/* C++ code produced by gperf version 3.0.4 */
/* Command-line: gperf -CGD -N PaloValue -K option -L C++ -t PaloCommands.gperf  */
/* Computed positions: -k'1,8,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "PaloCommands.gperf"

#include "palo.h"

#include "PaloHttpServer/PaloRequestHandler.h"
#line 6 "PaloCommands.gperf"
struct CommandOption {
  const char * option;
  int code;
};

#define TOTAL_KEYWORDS 81
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 178
/* maximum key range = 174, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct CommandOption *PaloValue (const char *str, unsigned int len);
};

inline unsigned int
Perfect_Hash::hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179,  95, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179,  55,  40,  45, 179,
      179, 179, 179, 179, 179, 179, 179,   5, 179, 179,
      179, 179, 179,  35, 179, 179,  35, 179,  35, 179,
      179, 179, 179, 179, 179, 179, 179,  25,  15,  10,
       30,  15,  85, 179,   0,  75, 179,  40,  65,   0,
        0,  40,   5, 179,  55,   0,  15,  60,  30,   0,
      179,  15,  60, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179, 179, 179, 179, 179,
      179, 179, 179, 179, 179, 179
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct CommandOption wordlist[] =
  {
#line 62 "PaloCommands.gperf"
    {"steps", palo::PaloRequestHandler::CMD_NUM_STEPS},
#line 77 "PaloCommands.gperf"
    {"splash", palo::PaloRequestHandler::CMD_SPLASH},
#line 82 "PaloCommands.gperf"
    {"weights", palo::PaloRequestHandler::CMD_WEIGHTS},
#line 40 "PaloCommands.gperf"
    {"path", palo::PaloRequestHandler::CMD_ID_PATH},
#line 41 "PaloCommands.gperf"
    {"paths", palo::PaloRequestHandler::CMD_ID_PATHS},
#line 72 "PaloCommands.gperf"
    {"show_system", palo::PaloRequestHandler::CMD_SHOW_SYSTEM},
#line 64 "PaloCommands.gperf"
    {"position", palo::PaloRequestHandler::CMD_POSITION},
#line 53 "PaloCommands.gperf"
    {"name_dimension", palo::PaloRequestHandler::CMD_NAME_DIMENSION},
#line 54 "PaloCommands.gperf"
    {"name_dimensions", palo::PaloRequestHandler::CMD_NAME_DIMENSIONS},
#line 27 "PaloCommands.gperf"
    {"children", palo::PaloRequestHandler::CMD_ID_CHILDREN},
#line 38 "PaloCommands.gperf"
    {"mode", palo::PaloRequestHandler::CMD_ID_MODE},
#line 45 "PaloCommands.gperf"
    {"types", palo::PaloRequestHandler::CMD_ID_TYPES},
#line 76 "PaloCommands.gperf"
    {"source", palo::PaloRequestHandler::CMD_SOURCE},
#line 46 "PaloCommands.gperf"
    {"machine", palo::PaloRequestHandler::CMD_MACHINE},
#line 33 "PaloCommands.gperf"
    {"elements", palo::PaloRequestHandler::CMD_ID_ELEMENTS},
#line 57 "PaloCommands.gperf"
    {"name_path", palo::PaloRequestHandler::CMD_NAME_PATH},
#line 58 "PaloCommands.gperf"
    {"name_paths", palo::PaloRequestHandler::CMD_NAME_PATHS},
#line 39 "PaloCommands.gperf"
    {"parent", palo::PaloRequestHandler::CMD_ID_PARENT},
#line 56 "PaloCommands.gperf"
    {"name_elements", palo::PaloRequestHandler::CMD_NAME_ELEMENTS},
#line 28 "PaloCommands.gperf"
    {"cube", palo::PaloRequestHandler::CMD_ID_CUBE},
#line 75 "PaloCommands.gperf"
    {"skip_empty", palo::PaloRequestHandler::CMD_SKIP_EMPTY},
#line 13 "PaloCommands.gperf"
    {"action", palo::PaloRequestHandler::CMD_ACTION,},
#line 17 "PaloCommands.gperf"
    {"comment", palo::PaloRequestHandler::CMD_COMMENT},
#line 66 "PaloCommands.gperf"
    {"sid", palo::PaloRequestHandler::CMD_SID},
#line 44 "PaloCommands.gperf"
    {"type", palo::PaloRequestHandler::CMD_ID_TYPE},
#line 23 "PaloCommands.gperf"
    {"event", palo::PaloRequestHandler::CMD_EVENT},
#line 81 "PaloCommands.gperf"
    {"values", palo::PaloRequestHandler::CMD_VALUES},
#line 32 "PaloCommands.gperf"
    {"element", palo::PaloRequestHandler::CMD_ID_ELEMENT},
#line 61 "PaloCommands.gperf"
    {"new_name", palo::PaloRequestHandler::CMD_NEW_NAME},
#line 51 "PaloCommands.gperf"
    {"name_cube", palo::PaloRequestHandler::CMD_NAME_CUBE},
#line 55 "PaloCommands.gperf"
    {"name_element", palo::PaloRequestHandler::CMD_NAME_ELEMENT},
#line 52 "PaloCommands.gperf"
    {"name_database", palo::PaloRequestHandler::CMD_NAME_DATABASE},
#line 67 "PaloCommands.gperf"
    {"show_attribute", palo::PaloRequestHandler::CMD_SHOW_ATTRIBUTE},
#line 11 "PaloCommands.gperf"
    {"actcode", palo::PaloRequestHandler::CMD_ACTCODE},
#line 18 "PaloCommands.gperf"
    {"complete", palo::PaloRequestHandler::CMD_COMPLETE},
#line 49 "PaloCommands.gperf"
    {"name_area", palo::PaloRequestHandler::CMD_NAME_AREA},
#line 80 "PaloCommands.gperf"
    {"value", palo::PaloRequestHandler::CMD_VALUE},
#line 85 "PaloCommands.gperf"
    {"expand", palo::PaloRequestHandler::CMD_EXPAND},
#line 42 "PaloCommands.gperf"
    {"path_to", palo::PaloRequestHandler::CMD_ID_PATH_TO},
#line 26 "PaloCommands.gperf"
    {"area", palo::PaloRequestHandler::CMD_ID_AREA},
#line 14 "PaloCommands.gperf"
    {"add", palo::PaloRequestHandler::CMD_ADD},
#line 19 "PaloCommands.gperf"
    {"condition", palo::PaloRequestHandler::CMD_CONDITION},
#line 12 "PaloCommands.gperf"
    {"activate", palo::PaloRequestHandler::CMD_ACTIVATE},
#line 83 "PaloCommands.gperf"
    {"show_lock_info", palo::PaloRequestHandler::CMD_SHOW_LOCK_INFO},
#line 21 "PaloCommands.gperf"
    {"extern_password", palo::PaloRequestHandler::CMD_EXTERN_PASSWORD},
#line 59 "PaloCommands.gperf"
    {"name_path_to", palo::PaloRequestHandler::CMD_NAME_PATH_TO},
#line 29 "PaloCommands.gperf"
    {"database", palo::PaloRequestHandler::CMD_ID_DATABASE},
#line 73 "PaloCommands.gperf"
    {"show_user_info", palo::PaloRequestHandler::CMD_SHOW_USER_INFO},
#line 63 "PaloCommands.gperf"
    {"password", palo::PaloRequestHandler::CMD_PASSWORD},
#line 43 "PaloCommands.gperf"
    {"rule", palo::PaloRequestHandler::CMD_ID_RULE},
#line 71 "PaloCommands.gperf"
    {"show_rules", palo::PaloRequestHandler::CMD_SHOW_RULES},
#line 30 "PaloCommands.gperf"
    {"dimension", palo::PaloRequestHandler::CMD_ID_DIMENSION},
#line 31 "PaloCommands.gperf"
    {"dimensions", palo::PaloRequestHandler::CMD_ID_DIMENSIONS},
#line 37 "PaloCommands.gperf"
    {"locked_paths", palo::PaloRequestHandler::CMD_ID_LOCKED_PATHS},
#line 79 "PaloCommands.gperf"
    {"use_rules", palo::PaloRequestHandler::CMD_USE_RULES},
#line 35 "PaloCommands.gperf"
    {"limit", palo::PaloRequestHandler::CMD_ID_LIMIT},
#line 34 "PaloCommands.gperf"
    {"lickey", palo::PaloRequestHandler::CMD_LICKEY},
#line 68 "PaloCommands.gperf"
    {"show_gputype", palo::PaloRequestHandler::CMD_SHOW_GPUTYPE},
#line 50 "PaloCommands.gperf"
    {"name_children", palo::PaloRequestHandler::CMD_NAME_CHILDREN},
#line 70 "PaloCommands.gperf"
    {"show_rule", palo::PaloRequestHandler::CMD_SHOW_RULE},
#line 65 "PaloCommands.gperf"
    {"properties", palo::PaloRequestHandler::CMD_PROPERTIES},
#line 84 "PaloCommands.gperf"
    {"function", palo::PaloRequestHandler::CMD_FUNCTION},
#line 25 "PaloCommands.gperf"
    {"functions", palo::PaloRequestHandler::CMD_FUNCTIONS},
#line 88 "PaloCommands.gperf"
    {"X-PALO-DIM", palo::PaloRequestHandler::CMD_X_PALO_DIMENSION},
#line 16 "PaloCommands.gperf"
    {"blocksize", palo::PaloRequestHandler::CMD_BLOCKSIZE},
#line 15 "PaloCommands.gperf"
    {"base_only", palo::PaloRequestHandler::CMD_BASE_ONLY},
#line 36 "PaloCommands.gperf"
    {"lock", palo::PaloRequestHandler::CMD_ID_LOCK},
#line 86 "PaloCommands.gperf"
    {"X-PALO-SV", palo::PaloRequestHandler::CMD_X_PALO_SERVER},
#line 20 "PaloCommands.gperf"
    {"definition", palo::PaloRequestHandler::CMD_DEFINITION},
#line 60 "PaloCommands.gperf"
    {"user", palo::PaloRequestHandler::CMD_NAME_USER},
#line 47 "PaloCommands.gperf"
    {"required", palo::PaloRequestHandler::CMD_REQUIRED},
#line 90 "PaloCommands.gperf"
    {"X-PALO-CC", palo::PaloRequestHandler::CMD_X_PALO_CUBE_CLIENT_CACHE},
#line 78 "PaloCommands.gperf"
    {"use_identifier", palo::PaloRequestHandler::CMD_USE_IDENTIFIER},
#line 69 "PaloCommands.gperf"
    {"show_normal", palo::PaloRequestHandler::CMD_SHOW_NORMAL},
#line 74 "PaloCommands.gperf"
    {"show_info", palo::PaloRequestHandler::CMD_SHOW_INFO},
#line 89 "PaloCommands.gperf"
    {"X-PALO-CB", palo::PaloRequestHandler::CMD_X_PALO_CUBE},
#line 24 "PaloCommands.gperf"
    {"event_processor", palo::PaloRequestHandler::CMD_EVENT_PROCESSOR},
#line 87 "PaloCommands.gperf"
    {"X-PALO-DB", palo::PaloRequestHandler::CMD_X_PALO_DATABASE},
#line 91 "PaloCommands.gperf"
    {"Content-Length", palo::PaloRequestHandler::CMD_CONTENT_LENGTH},
#line 22 "PaloCommands.gperf"
    {"external_identifier", palo::PaloRequestHandler::CMD_EXTERNAL_IDENTIFIER},
#line 48 "PaloCommands.gperf"
    {"optional", palo::PaloRequestHandler::CMD_OPTIONAL}
  };

static const signed char lookup[] =
  {
    -1, -1, -1, -1, -1,  0,  1,  2, -1,  3,  4,  5, -1,  6,
     7,  8, -1, -1,  9, 10, 11, 12, 13, 14, 15, 16, 17, -1,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, -1, -1,
    30, 31, 32, -1, -1, 33, 34, 35, 36, 37, 38, -1, 39, -1,
    -1, -1, 40, 41, -1, -1, -1, 42, 43, 44, -1, 45, 46, 47,
    -1, -1, -1, 48, 49, 50, -1, -1, -1, 51, 52, -1, 53, -1,
    54, 55, 56, 57, 58, 59, 60, -1, -1, 61, 62, 63, -1, -1,
    -1, 64, -1, -1, -1, -1, 65, -1, -1, -1, -1, 66, -1, -1,
    -1, -1, 67, 68, -1, -1, -1, 69, -1, -1, -1, 70, 71, -1,
    -1, -1, -1, 72, -1, 73, -1, -1, 74, -1, -1, -1, -1, 75,
    76, -1, -1, -1, 77, -1, -1, -1, -1, 78, -1, -1, -1, -1,
    79, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 80
  };

const struct CommandOption *
Perfect_Hash::PaloValue (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].option;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
