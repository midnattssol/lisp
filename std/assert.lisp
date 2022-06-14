(=> assert_expr_eq [expr expected] (do
    (= result (eval_expr expr))
    (if! (!= result expected) (do
        (put "Expected result of `" expr "` to be equal to `" expected "`, but got `" result "`.")
        (exit 1)
    ))
))
