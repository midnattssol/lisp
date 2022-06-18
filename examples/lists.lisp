; Get all numbers between 0 and 50.
(= numbers :50) ; -> [0 ... 49]

; Get an element from the list.
(@ 5 numbers) ; -> 5
(get 5 numbers) ; -> 5

; Get the first element from the list.
(head! numbers) ; -> 0

; Get the last element from the list.
(tail! numbers) ; -> 49

; Get the length of the list.
(# numbers) ; -> 50
(len numbers) ; -> 50

; Reverse the list.
(rev! numbers) ; -> [49 ... 0]
($ numbers -1 0 -1) ; -> [49 ... 0]
(slice numbers -1 0 -1) ; -> [49 ... 0]

; Get elements with indices ranging between 10 and 20.
($ numbers 10 20) ; -> [10 ... 19]
(slice numbers 10 20) ; -> [49 ... 0]

; Join lists.
( :3 :5 :3) ; -> [0 1 2 0 1 2 3 4 0 1 2]
(join :3 :5 :3) ; -> [0 1 2 0 1 2 3 4 0 1 2]

; Get a list from a single element.
(repeat 4 10) ; -> [10 10 10 10]

; Construct and flatten the threefold repetition of [0 1 2].
(/ , (repeat 3 :3)) ; -> [0 1 2 0 1 2 0 1 2]
