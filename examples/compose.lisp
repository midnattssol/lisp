(use! "functional")

(= add_10_times_2 (..> #[+ _ 10] #[* _ 2]))
(putl! (add_10_times_2 10))
