(use! "functional")


; Convolves two lists together.
; For an intuitive explanation of a convolution, see
; https://betterexplained.com/articles/intuitive-convolution.
(=> convolve_lists [kernel signal] (do
    (= kernel (slice kernel -1 0 -1))
    (= n (# kernel))
    (= out [])

    ; Pad the signal.
    (= signal (, (repeat (- n 1) 0) signal (repeat n 0)))

    (= l (- (# signal) n))
    (= i 0)

    (while! (< i l) (do
        (= window (slice signal i (+ i n -1)))
        (push out (/ + (map * kernel window)))
        (++ i)
    ))

    out
))

; Return all items of the resulting convolution where the kernel and signal
; overlap completely, so no boundaries are visible.
(=> convolve_lists_valid [kernel signal] (do
    (= offset (// (# kernel) 2))
    (+= offset 1)
    (slice (convolve_lists kernel signal) offset (neg (+ offset 1)))
))


(= result (convolve_lists_valid (repeat 3 (// 1. 3.)) [1 2 3 4 1]))
(put result)
