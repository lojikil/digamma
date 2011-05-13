#! /usr/bin/env vesta
; a simple lab note software package
; the original interface is command driven, but will eventually be moved to multi-access
; with Web, JSON, GUI & CLI versions.
; released under zlib/png license, and (c) 2010 Stefan Edwards
;

(use "strlib")

(def *base-dir* (tilde-expand "~/.digamma/anaxagoras"))
(def *notes* {})
(defn prompt-string (s)
 (display s)
 (read-string))
(defn int-read () ; read until "^\.$"
 (with r (read-string)
  (cond
   (eq? r ".") '()
   (eq? r #e) '()
   else (cons r (int-read)))))
(defn new-note () 
 (let ((stamp (sys :time)) 
       (title (prompt-string "title: "))
       (data (int-read)))
  (let ((f (open (format "~s/~a" *base-dir* stamp) :write)))
   (display (format "~s\n" title) f)
   (display (string-join data "\n") f)
   (newline f)
   (close f))))
(defn new-url ()
 (display "New URL.\n"))
(defn list-notes ()
 (display "List Notes.\n"))
(defn rebuild-db ()
 (display "Rebuild.\n"))
(defn about-anaxagoras ()
 (display "Anaxagoras is a simple Note & URL organzier. It is named \"anaxagoras\" in honor of the 
 Pre-Socratic philosopher, who placed \"Nous\" (mind) as the organizing principle of Reality.
 Usage:
  n[ew]     - create a new note
  u[rl]     - create a new URL bookmark
  l[ist]    - list all notes
  a[bout]   - show this message
  h[elp]    - show this message
  ?         - show this message
  r[ebuild] - rebuild index from messages
  q[uit]    - exit anaxagoras\n"))
(defn anaxagoras ()
 (display "> ")
 (with c (read-string)
  (begin
    (cond ; case macro would be useful here...
        (or (eq? c "n") (eq? c "N") (eq? c "new")) (new-note)
        (or (eq? c "u") (eq? c "U") (eq? c "url")) (new-url)
        (or (eq? c "l") (eq? c "L") (eq? c "list")) (list-notes)
        (or (eq? c "a") (eq? c "A") (eq? c "about") (eq? c "h") (eq? c "H") (eq? c "help")) 
            (about-anaxagoras)
        (or (eq? c "r") (eq? c "R") (eq? c "rebuild")) (rebuild-db)
        (or (eq? c "q") (eq? c "Q") (eq? c "quit")) #v
        else (display "Invalid command\n"))
    (anaxagoras))))
(display "Welcome to anaxagoras, a note/url organizer\n")
(anaxagoras)
