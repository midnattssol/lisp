(use! "functional")

; https://code.golf/divisors
(=> divisors (/? (-> [x] (! (% _ x))) 1:(+ _ 1)))
(. #[putl! ($ (repr (divisors _)) 1 -2)] 1:101)
