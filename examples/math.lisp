(use! "math")
(use! "io")

(putl (<< 1 8)) ; -> 256
(putl (** 2 5)) ; -> 32
(putl (factorial 10)) ; -> 3628800
(putl (sgn -10)) ; -> -1
(putl (abs -289348)) ; -> 289348


; A switch/case implementation of the Fibonacci sequence.
(=> fib
    (switch _
        (case 0 1)
        (case 1 1)
        (otherwise (+ (fib (-- _)) (fib (-- _))))
    )
)

; Get prime factors of a number.
(=> factors (do
    (= i 2)
    (while! (leq (* i i) _) (do
        (unless! (% _ i) (return (push! i (factors (// _ i)))))
        (++ i)
    ))
    [_]
))

; Gets whether or not a number is prime.
(=> prime? (do
    (= i 2)
    (while! (leq (* i i) _) (do
        (unless! (% _ i) (return No))
        (++ i)
    ))
    Yes
))

(=> triangle_number (/ + :(++ _) 0))

; An example of a closure returning a closure.
(=> add_n [n] #[+ _ n])
(= add_10 (add_n 10))
(put (add_10 3))


(putl! (. triangle_number :15))
(putl! (. prime? :15))
(putl! (. fib :15))
