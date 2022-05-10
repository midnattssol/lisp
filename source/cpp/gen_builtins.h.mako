/* This code was automatically generated from `${filename.name}`. */
#include <map>
#include <string>
#include <set>

enum LispBuiltin {
    % for signature in SIGNATURES:
    ${_builtin2enum(signature)},
    % endfor
};

const std::map<LispBuiltin, std::string> BUILTINS_NAMES = {
    % for signature in SIGNATURES:
    {${_builtin2enum(signature)}, ${f'"{signature}"'}},
    % endfor
};

const std::map<std::string, LispBuiltin> BUILTINS_NUMS = {
    % for signature in SIGNATURES:
    {${f'"{signature}"'}, ${_builtin2enum(signature)}},
    % endfor
};

const std::set<std::string> LISP_BUILTINS = {
    % for signature in SIGNATURES:
    ${f'"{signature}"'},
    % endfor
};
