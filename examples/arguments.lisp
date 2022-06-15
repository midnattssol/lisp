; Expects to be passed arguments of integers.
(= args ($ argv 1))
(.= args parse)

(putl! "Arguments: " args)
(putl! "Sum: " (/ + args))
(putl! "Average: " (// (/ + args) (float (# args))))
