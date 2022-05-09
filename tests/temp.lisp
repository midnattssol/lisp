(use! "assert")
(use! "functional")


(assert_expr_eq {(?> + 10)} 10)
(assert_expr_eq {(?> + Nothing)} Nothing)
