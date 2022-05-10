/* This code was automatically generated from `${filename.name}`. */
#include <map>
#include "./lispvar.h"

std::map<std::string, LispVar *> BUILTINS_TYPES = {};
bool BUILTINS_TYPES_READY = false;

void _fill_out_lisp_builtin_types() {
    *_SINGLETON_NOTHING = {NOTHING, 0};
    *_SINGLETON_NOT_SET = {__NOT_SET__, 0};
    *_SINGLETON_NOARGS_TOKEN = {__NO_ARGS__, 0};
    % for i, _ in enumerate(UNIQUE_SIGNATURES):
    auto type_${i} = new LispVar;
    % endfor

    % for i, sign in enumerate(UNIQUE_SIGNATURES):
    *type_${i} = parse_and_evaluate("${_escape(sign)}");
    % endfor

    % for builtin, (signature, *_) in SIGNATURES.items():
    BUILTINS_TYPES["${builtin}"] = type_${UNIQUE_SIGNATURES.index(signature)};
    %endfor

    BUILTINS_TYPES_READY = true;
}
