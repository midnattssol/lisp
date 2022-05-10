# Builtins

This page has documentation about the builtins of the Lisp.

% for name, items in SIGNATURES.items():
${"#" * 2} `${name}`
_Signature: `${_parse_signature(items[0])} -> ${items[1]}`_

${items[2]}

% if (len(items) >= 4):
${"#" * 3} Examples

    ${items[3]}

% endif
% endfor
