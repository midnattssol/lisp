(use! "functional")

; https://code.golf/leap-years
(= leap-year? #[& (! (% _ 4)) (% _ 400)])
(. #[putl! _] (/? leap-year? 1800:2401))
