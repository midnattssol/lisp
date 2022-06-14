; Get all numbers between 0 and 50.
(= numbers :50) ; -> [0 ... 49]

; Get the first element from the list.
(@ 0 numbers) ; -> 0

; Get the last element from the list.
(@ -1 numbers) ; -> 49

; Get the length of the list.
(# numbers) ; -> 50

; Reverse the list.
($ numbers -1 0 -1) ; -> [49 .. 0]

; Get elements with indices ranging between 10 and 20.
($ numbers 10 20) ; -> [10 .. 19]

; Join lists.
(, :3 :5 :3) ; -> [0, 1, 2, 0, 1, 2, 3, 4, 0, 1, 2]

; Get a list from a single element.
(repeat 4 10) ; -> [10, 10, 10, 10]
