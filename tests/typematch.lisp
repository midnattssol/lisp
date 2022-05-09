(use! "assert")

; Test the type matching.
(assert_expr_eq {(typematch [[(type "int") (type "*")]] [1 2 3])} Yes)
(assert_expr_eq {(typematch [[(type "int") (type "+")]] [1 2 3])} Yes)
(assert_expr_eq {(typematch [[(type "int") (type "+")]] [])} No)
