; Insert an item into a sorted list
; in such a way that the list remains sorted.
(=> _insert_into_sorted [item _list] (do
    (= l (# _list))
    (= i (- l 1))
    (= ptr l)

    (while! (>= i 0) (do
        ; Set the position to the spot if it's less than or equal to the item.
        (= ptr (? (> (@ i _list) item) i ptr))
        (-- i)
    ))

    (linsert item ptr _list)
))


; Get a sorted copy of a list.
(=> insertion_sort [_list] (do
    (= out l[])
    (= i 0)
    (= l (# _list))

    (while! (< i l) (do
        (= out (_insert_into_sorted (@ i _list) out))
        (++ i)
    ))

    out
))


; Sorts 512 numbers 4 times.
(insertion_sort (rand 100))
; (insertion_sort (rand 512))
; (insertion_sort (rand 512))
; (insertion_sort (rand 512))
