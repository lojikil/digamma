#! /usr/bin/env vesta
; a simple lab note software package
; the original interface is command driven, but will eventually be moved to multi-access
; with Web, JSON, GUI & CLI versions.
; released under zlib/png license, and (c) 2010 Stefan Edwards

(def *base-dir* (tilde-expand "~/.anaxagoras"))
(def *notes* {})
(defn new-note () ; read until "^.$"
 (display "New Note.\n"))
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
  r[ebuild] - rebuild index from messages
  q[uit]    - exit anaxagoras\n"))
(defn anaxagoras ()
 (display "> ")
 (with c (read-string)
  (cond ; case macro would be useful here...
   (or (eq? c "n") (eq? c "N") (eq? c "new"))
    (begin (new-note) (anaxagoras))
   (or (eq? c "u") (eq? c "U") (eq? c "url"))
    (begin (new-url) (anaxagoras))
   (or (eq? c "l") (eq? c "L") (eq? c "list"))
    (begin (list-notes) (anaxagoras))
   (or (eq? c "a") (eq? c "A") (eq? c "about"))
    (begin (about-anaxagoras) (anaxagoras))
   (or (eq? c "r") (eq? c "R") (eq? c "rebuild"))
    (begin (rebuild-db) (anaxagoras))
   (or (eq? c "q") (eq? c "Q") (eq? c "quit"))
    #v
   else
    (begin (display "Invalid command\n") (anaxagoras)))))
(display "Welcome to anaxagoras, a note/url organizer\n")
(anaxagoras)
