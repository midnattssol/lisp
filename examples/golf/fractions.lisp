; https://code.golf/fractions

; Assumes that (<= a b).
(=> gcd [a b] (do
    (unless! a (return b))
    (= b' (% b a))
    (if! (< a b') (gcd a b') (gcd b' a))
))

; Reduce a fraction such as "99/18" and return
; a list of 2 items.
(=> reduce_fraction [frac] (do
    (= frac (split "/" frac))
    (.= frac parse)

    (= sorted (if! (/ > frac) (rev! frac) frac))
    (= gcd_ (apply gcd sorted))
    (. #[// _ gcd_] frac)
))


(= fractions ($ argv 1))
(.= fractions reduce_fraction)
(. #[putl! (@ 0 _) "/" (@ 1 _)] fractions)
