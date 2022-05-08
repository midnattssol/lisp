(=> ! (? _ False True))

(=> assert_expr_eq [expr expected] (do
    (= result (eval_expr expr))
    (if! (!= result expected) (do
        (map put ["Expected result of `" expr "` to be equal to `" expected "`, but got `" result "`."])
        (exit 1)
    ) Nothing )
))


; Test the type matching.
(assert_expr_eq {(typematch [[(type "int") (type "*")]] [1 2 3])} Yes)
(assert_expr_eq {(typematch [[(type "int") (type "+")]] [1 2 3])} Yes)
(assert_expr_eq {(typematch [[(type "int") (type "+")]] [])} No)
