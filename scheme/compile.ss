(load 'Eprime.ss)

(if (= (length *command-line*) 3)
    (eprime (nth *command-line* 0) (nth *command-line* 1) (nth *command-line* 2))
    (display "usage: compile.ss infile outfile init_name\n"))
