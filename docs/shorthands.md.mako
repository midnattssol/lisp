${"#" * 2} Shorthands

| Item | Name |
| :-- | :-- |
% for item, name in SHORTHANDS.items():
| `${item.replace("|", "\\|")}` | `${name.replace("|", "\\|")}` |
% endfor

${"#" * 2} In-place assignments

| Item | Expansion |
| :-- | :-- |
% for item, name in PREFIX_EQUALS.items():
| `${item.replace("|", "\\|")}` | `${name["op"].replace("|", "\\|")}` |
% endfor
