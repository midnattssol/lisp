# A CSON grammar for Atom.

'scopeName': 'source.my-lisp'
'name': 'MyLisp'
'fileTypes': [
    'lisp'
]
'patterns': [
    {
        match: ';.*'
        name: 'comment'
    }
    {
        match: '[\\(\\{\\[\\]\\}\\):]'
        name: 'punctuation'
    }
    {
        # This is actually a macro, but I don't know the proper name
        # And the highlighting looks nice with this
        match: '\\w+!'
        name: 'support.function.builtin'
    }
    {
        match: '${_regex_joined(PREFIX_EQUALS, "|")}'
        name: 'storage'
    }
    {
        match: '(?<!\\w)(${_regex_joined([i for i in SHORTHANDS if i != "="], "|")})(?!\\w)'
        name: 'keyword.operator'
    }
    {
        match: '${_regex_joined(("=>", "λ", "=", "let"), "|")}'
        name: 'storage'
    }
    {
        match: '(\\d+\\.\\d*)|(\\d*\\.\\d+)'
        name: 'constant.numeric.float'
    }
    {
        match: '\\b-?(0[xt])?[0-9_]+\\b'
        name: 'constant.numeric.integer'
    }
    {
        match: '\\b0x[0-9a-fA-F_]+\\b'
        name: 'constant.numeric.hex'
    }
    {
        match: '(["])((?:\\\\\\1|(?!\\1).)+)\\1"'
        name: 'string'
    }
    {
        name: 'string.quoted.double'
        begin: '"'
        end: '"'
        patterns: (
            {
                name : 'constant.character.escape'
                match : '\\.'
            }
        )
    }
    {
        match: '\\b(Nothing|Nil|True|False|Yes|No|On|Off)\\b'
        name: 'constant'
    }
    {
        match: '\\b(${_regex_joined([i for i in SIGNATURES if i != "let"], "|")})\\b'
        name: 'entity.name.function'
    }
    {
        match: '\\w(\\w|[-_+?])*'
        name: 'variable'
    }
]
