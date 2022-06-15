(use! "functional")

; https://code.golf/niven-numbers
(. #[putl! _] (/? #[! (% _ (/ + (. parse ($ (split "" (repr _)) 1))))] 1:101))
