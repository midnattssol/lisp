; An example of a closure returning a closure.
(=> add_n [n] (lambda! [x] (+ x n)))

(= add10 (add_n 10))
(put (add10 3))
