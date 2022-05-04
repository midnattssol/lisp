/* This code was automatically generated. */
#include <set>
#include <map>
#include "./lispvar.h"
std::map<std::string, LispVar *> BUILTINS_TYPES = {};
bool BUILTINS_TYPES_READY = false;
void _fill_out_lisp_builtin_types() {
*_SINGLETON_NOTHING = {NOTHING, 0};
*_SINGLETON_NOT_SET = {__NOT_SET__, 0};
*_SINGLETON_NOARGS_TOKEN = {__NO_ARGS__, 0};
auto type_0 = new LispVar;
*type_0 = parse_and_evaluate("[(map type ['*' 'numeric'])]");
BUILTINS_TYPES["mul"] = type_0;
BUILTINS_TYPES["add"] = type_0;
BUILTINS_TYPES["or"] = type_0;
BUILTINS_TYPES["and"] = type_0;
BUILTINS_TYPES["xor"] = type_0;
BUILTINS_TYPES["gt"] = type_0;
BUILTINS_TYPES["geq"] = type_0;
BUILTINS_TYPES["lt"] = type_0;
BUILTINS_TYPES["leq"] = type_0;
auto type_1 = new LispVar;
*type_1 = parse_and_evaluate("[(map type ['*'])]");
BUILTINS_TYPES["noop"] = type_1;
BUILTINS_TYPES["list"] = type_1;
BUILTINS_TYPES["eq"] = type_1;
BUILTINS_TYPES["neq"] = type_1;
auto type_2 = new LispVar;
*type_2 = parse_and_evaluate("[(map type ['any'])]");
BUILTINS_TYPES["typeof"] = type_2;
BUILTINS_TYPES["symbol"] = type_2;
BUILTINS_TYPES["repr"] = type_2;
BUILTINS_TYPES["put"] = type_2;
BUILTINS_TYPES["help"] = type_2;
auto type_3 = new LispVar;
*type_3 = parse_and_evaluate("[(map type ['list'])]");
BUILTINS_TYPES["len"] = type_3;
BUILTINS_TYPES["eval"] = type_3;
auto type_4 = new LispVar;
*type_4 = parse_and_evaluate("[(map type ['string'])]");
BUILTINS_TYPES["type"] = type_4;
BUILTINS_TYPES["resolve"] = type_4;
auto type_5 = new LispVar;
*type_5 = parse_and_evaluate("[(map type ['*' 'list'])]");
BUILTINS_TYPES["join"] = type_5;
auto type_6 = new LispVar;
*type_6 = parse_and_evaluate("[(map type ['numeric'])]");
BUILTINS_TYPES["neg"] = type_6;
BUILTINS_TYPES["flip"] = type_6;
auto type_7 = new LispVar;
*type_7 = parse_and_evaluate("[(map type ['numeric']) (map type ['numeric'])]");
BUILTINS_TYPES["mod"] = type_7;
BUILTINS_TYPES["sub"] = type_7;
BUILTINS_TYPES["div"] = type_7;
auto type_8 = new LispVar;
*type_8 = parse_and_evaluate("[(map type ['numeric' '?']) (map type ['numeric' '?']) (map type ['numeric' '?' 'truthy'])]");
BUILTINS_TYPES["range"] = type_8;
auto type_9 = new LispVar;
*type_9 = parse_and_evaluate("[(map type ['list']) (map type ['numeric']) (map type ['any'])]");
BUILTINS_TYPES["insert"] = type_9;
auto type_10 = new LispVar;
*type_10 = parse_and_evaluate("[(map type ['booly']) (map type ['string' '?'])]");
BUILTINS_TYPES["assert"] = type_10;
auto type_11 = new LispVar;
*type_11 = parse_and_evaluate("[(map type ['booly'])]");
BUILTINS_TYPES["bool"] = type_11;
auto type_12 = new LispVar;
*type_12 = parse_and_evaluate("[(map type ['list']) (map type ['numeric']) (map type ['numeric' '?']) (map type ['numeric' '?' 'truthy'])]");
BUILTINS_TYPES["slice"] = type_12;
auto type_13 = new LispVar;
*type_13 = parse_and_evaluate("[(map type ['numeric']) (map type ['any'])]");
BUILTINS_TYPES["repeat"] = type_13;
auto type_14 = new LispVar;
*type_14 = parse_and_evaluate("[(map type ['numeric']) (map type ['list'])]");
BUILTINS_TYPES["get"] = type_14;
auto type_15 = new LispVar;
*type_15 = parse_and_evaluate("[(map type ['list']) (map type ['list'])]");
BUILTINS_TYPES["typematch"] = type_15;
auto type_16 = new LispVar;
*type_16 = parse_and_evaluate("[(map type ['builtin']) (map type ['list' '*'])]");
BUILTINS_TYPES["map"] = type_16;
auto type_17 = new LispVar;
*type_17 = parse_and_evaluate("[(map type ['builtin']) (map type ['any']) (map type ['list'])]");
BUILTINS_TYPES["fold"] = type_17;
auto type_18 = new LispVar;
*type_18 = parse_and_evaluate("[(map type ['booly']) (map type ['any']) (map type ['any'])]");
BUILTINS_TYPES["ternary"] = type_18;
auto type_19 = new LispVar;
*type_19 = parse_and_evaluate("[(map type ['string']) (map type ['any'])]");
BUILTINS_TYPES["let"] = type_19;
auto type_20 = new LispVar;
*type_20 = parse_and_evaluate("[(map type ['expression']) (map type ['any' '*'])]");
BUILTINS_TYPES["call"] = type_20;
BUILTINS_TYPES_READY = true;
}
const std::set<std::string> LISP_BUILTINS = {"eq",
"gt",
"lt",
"or",
"add",
"and",
"div",
"geq",
"get",
"len",
"leq",
"let",
"map",
"mod",
"mul",
"neg",
"neq",
"put",
"sub",
"xor",
"bool",
"call",
"eval",
"flip",
"fold",
"help",
"join",
"list",
"noop",
"repr",
"type",
"range",
"slice",
"assert",
"insert",
"repeat",
"symbol",
"typeof",
"resolve",
"ternary",
"typematch"};
