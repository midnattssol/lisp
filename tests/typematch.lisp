; An example test.
(=> ! (? _ False True))

(assert (typematch [[(type "int") (type "*")]] [1 2 3]))
(assert (typematch [[(type "int") (type "+")]] [1 2 3]))
(assert (! (typematch [[(type "int") (type "+")]] [])))
