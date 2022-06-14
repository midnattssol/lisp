(=> fib (do
    (if! (leq _ 1) (return 1))
    (+ (fib (-- _)) (fib (-- _)))
))

(putl! (map fib :10))

(putl! .)
