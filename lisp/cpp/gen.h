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
*type_0 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["mul"] = type_0;
auto type_1 = new LispVar;
*type_1 = parse_and_evaluate("[(map type [\"int\"])]");
BUILTINS_TYPES["rand"] = type_1;
auto type_2 = new LispVar;
*type_2 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["add"] = type_2;
auto type_3 = new LispVar;
*type_3 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["or"] = type_3;
auto type_4 = new LispVar;
*type_4 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["and"] = type_4;
auto type_5 = new LispVar;
*type_5 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["xor"] = type_5;
auto type_6 = new LispVar;
*type_6 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["gt"] = type_6;
auto type_7 = new LispVar;
*type_7 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["geq"] = type_7;
auto type_8 = new LispVar;
*type_8 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["lt"] = type_8;
auto type_9 = new LispVar;
*type_9 = parse_and_evaluate("[(map type [\"*\" \"numeric\"])]");
BUILTINS_TYPES["leq"] = type_9;
auto type_10 = new LispVar;
*type_10 = parse_and_evaluate("[(map type [\"*\"])]");
BUILTINS_TYPES["noop"] = type_10;
auto type_11 = new LispVar;
*type_11 = parse_and_evaluate("[(map type [\"*\"])]");
BUILTINS_TYPES["list"] = type_11;
auto type_12 = new LispVar;
*type_12 = parse_and_evaluate("[(map type [\"*\"])]");
BUILTINS_TYPES["eq"] = type_12;
auto type_13 = new LispVar;
*type_13 = parse_and_evaluate("[(map type [\"*\"])]");
BUILTINS_TYPES["neq"] = type_13;
auto type_14 = new LispVar;
*type_14 = parse_and_evaluate("[(map type [\"any\"])]");
BUILTINS_TYPES["typeof"] = type_14;
auto type_15 = new LispVar;
*type_15 = parse_and_evaluate("[(map type [\"any\"])]");
BUILTINS_TYPES["symbol"] = type_15;
auto type_16 = new LispVar;
*type_16 = parse_and_evaluate("[(map type [\"any\"])]");
BUILTINS_TYPES["repr"] = type_16;
auto type_17 = new LispVar;
*type_17 = parse_and_evaluate("[(map type [\"any\"])]");
BUILTINS_TYPES["put"] = type_17;
auto type_18 = new LispVar;
*type_18 = parse_and_evaluate("[(map type [\"any\"])]");
BUILTINS_TYPES["help"] = type_18;
auto type_19 = new LispVar;
*type_19 = parse_and_evaluate("[(map type [\"list\"])]");
BUILTINS_TYPES["len"] = type_19;
auto type_20 = new LispVar;
*type_20 = parse_and_evaluate("[(map type [\"list\"])]");
BUILTINS_TYPES["eval"] = type_20;
auto type_21 = new LispVar;
*type_21 = parse_and_evaluate("[(map type [\"expression\"])]");
BUILTINS_TYPES["eval_expr"] = type_21;
auto type_22 = new LispVar;
*type_22 = parse_and_evaluate("[(map type [\"string\"])]");
BUILTINS_TYPES["type"] = type_22;
auto type_23 = new LispVar;
*type_23 = parse_and_evaluate("[(map type [\"*\" \"list\"])]");
BUILTINS_TYPES["join"] = type_23;
auto type_24 = new LispVar;
*type_24 = parse_and_evaluate("[(map type [\"numeric\"])]");
BUILTINS_TYPES["neg"] = type_24;
auto type_25 = new LispVar;
*type_25 = parse_and_evaluate("[(map type [\"int\"])]");
BUILTINS_TYPES["flip"] = type_25;
auto type_26 = new LispVar;
*type_26 = parse_and_evaluate("[(map type [\"int\"]) (map type [\"int\"])]");
BUILTINS_TYPES["mod"] = type_26;
auto type_27 = new LispVar;
*type_27 = parse_and_evaluate("[(map type [\"numeric\"]) (map type [\"numeric\"])]");
BUILTINS_TYPES["sub"] = type_27;
auto type_28 = new LispVar;
*type_28 = parse_and_evaluate("[(map type [\"numeric\"]) (map type [\"numeric\"])]");
BUILTINS_TYPES["div"] = type_28;
auto type_29 = new LispVar;
*type_29 = parse_and_evaluate("[(map type [\"int\" \"?\"]) (map type [\"int\" \"?\"]) (map type [\"int\" \"?\" \"truthy\"])]");
BUILTINS_TYPES["range"] = type_29;
auto type_30 = new LispVar;
*type_30 = parse_and_evaluate("[(map type [\"any\"]) (map type [\"int\"]) (map type [\"list\"])]");
BUILTINS_TYPES["insert"] = type_30;
auto type_31 = new LispVar;
*type_31 = parse_and_evaluate("[(map type [\"booly\"]) (map type [\"string\" \"?\"])]");
BUILTINS_TYPES["assert"] = type_31;
auto type_32 = new LispVar;
*type_32 = parse_and_evaluate("[(map type [\"booly\"])]");
BUILTINS_TYPES["bool"] = type_32;
auto type_33 = new LispVar;
*type_33 = parse_and_evaluate("[(map type [\"list\"]) (map type [\"int\"]) (map type [\"int\" \"?\"]) (map type [\"int\" \"?\"])]");
BUILTINS_TYPES["slice"] = type_33;
auto type_34 = new LispVar;
*type_34 = parse_and_evaluate("[(map type [\"int\"]) (map type [\"any\"])]");
BUILTINS_TYPES["repeat"] = type_34;
auto type_35 = new LispVar;
*type_35 = parse_and_evaluate("[(map type [\"int\"]) (map type [\"list\"])]");
BUILTINS_TYPES["get"] = type_35;
auto type_36 = new LispVar;
*type_36 = parse_and_evaluate("[(map type [\"list\"]) (map type [\"list\"])]");
BUILTINS_TYPES["typematch"] = type_36;
auto type_37 = new LispVar;
*type_37 = parse_and_evaluate("[(map type [\"callable\"]) (map type [\"list\" \"*\"])]");
BUILTINS_TYPES["map"] = type_37;
auto type_38 = new LispVar;
*type_38 = parse_and_evaluate("[(map type [\"callable\"]) (map type [\"?\"]) (map type [\"list\"])]");
BUILTINS_TYPES["fold"] = type_38;
auto type_39 = new LispVar;
*type_39 = parse_and_evaluate("[(map type [\"booly\"]) (map type [\"any\"]) (map type [\"any\"])]");
BUILTINS_TYPES["ternary"] = type_39;
auto type_40 = new LispVar;
*type_40 = parse_and_evaluate("[(map type [\"string\"]) (map type [\"any\"])]");
BUILTINS_TYPES["let"] = type_40;
auto type_41 = new LispVar;
*type_41 = parse_and_evaluate("[(map type [\"callable\"]) (map type [\"any\" \"*\"])]");
BUILTINS_TYPES["call"] = type_41;
auto type_42 = new LispVar;
*type_42 = parse_and_evaluate("[(map type [\"expression\"])]");
BUILTINS_TYPES["closure"] = type_42;
auto type_43 = new LispVar;
*type_43 = parse_and_evaluate("[(map type [\"string\"]) (map type [\"string\"])]");
BUILTINS_TYPES["match"] = type_43;
auto type_44 = new LispVar;
*type_44 = parse_and_evaluate("[(map type [\"+\"])]");
BUILTINS_TYPES["do"] = type_44;
auto type_45 = new LispVar;
*type_45 = parse_and_evaluate("[(map type [\"expression\"]) (map type [\"expression\"])]");
BUILTINS_TYPES["while"] = type_45;
BUILTINS_TYPES_READY = true;
}
const std::set<std::string> LISP_BUILTINS = {"do",
"eq",
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
"rand",
"repr",
"type",
"match",
"range",
"slice",
"while",
"assert",
"insert",
"repeat",
"symbol",
"typeof",
"closure",
"ternary",
"eval_expr",
"typematch"};
