#! /usr/bin/env vesta
; a simple lab note software package
; the original interface is command driven, but will eventually be moved to multi-access
; with Web, JSON, GUI & CLI versions.
; released under zlib/png license, and (c) 2010 Stefan Edwards
;
; More ideas:
;  - add title & body search
;  - notes edited via internal ed-like editor or whatever is in $EDITOR
;  - List & Edit URLS, descriptions
;  - list notes & URLs, with paging
;  - ~/.digamma/anaxagoras/profile (simple dict)
;  - Server interface for uploading & syncing

(use "strlib")

(def *base-dir* (tilde-expand "~/.digamma/anaxagoras"))
(def *notes* {})
(def (prompt-string s)
 (display s)
 (read-string))
(def (int-read ) ; read until "^\.$"
 (with r (read-string)
  (cond
   (eq? r ".") '()
   (eq? r #e) '()
   else (cons r (int-read)))))
(def (new-note) 
 (let  ((f (open (format "~s/~a" *base-dir* (sys :time)) :write)) 
       (title (prompt-string "title: "))
       (data (int-read)))
   (display (format "~s\n" title) f)
   (display (string-join data "\n") f)
   (newline f)
   (close f)))
(def (new-url)
 (display "New URL.\n"))
(def (list-notes)
 (display "List Notes.\n"))
(def (rebuild-db)
 (display "Rebuild.\n"))
(def (about-anaxagoras)
 (display "Anaxagoras is a simple Note & URL organzier. It is named \"anaxagoras\" in honor of the 
 Pre-Socratic philosopher, who placed \"Nous\" (mind) as the organizing principle of Reality.
 Usage:
  n[ew]     - create a new note
  u[rl]     - create a new URL bookmark
  l[ist]    - list all notes
  h[elp]    - show this message
  ?         - show this message
  r[ebuild] - rebuild index from messages
  q[uit]    - exit anaxagoras\n"))
(def (anaxagoras)
 (display "> ")
 (with c (read-string)
  (begin
    (cond ; case macro would be useful here...
        (eq? c "n") (new-note)
        (eq? c "u") (new-url)
        (eq? c "l") (list-notes)
        (eq? c "h") (about-anaxagoras)
        (eq? c "r") (rebuild-db)
        (eq? c "q") #v
        else (display "Invalid command\n"))
    (anaxagoras))))
(display "Welcome to anaxagoras, a note/url organizer\n")
(anaxagoras)
