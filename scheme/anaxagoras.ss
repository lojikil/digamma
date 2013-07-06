#! /usr/bin/env vesta
; a simple lab note software package
; the original interface is command driven, but will eventually be moved to multi-access
; with Web, JSON, GUI & CLI versions.
; released under zlib/png license, and (c) 2010 Stefan Edwards
;
; More ideas:
;  - add title & body search (MAJORLY important)
;  - notes edited via internal ed-like editor or whatever is in $EDITOR
;  - List & Edit URLS, descriptions
;  - list notes & URLs, with paging
;  - ~/.digamma/anaxagoras/profile (simple dict)
;  - Server interface for uploading & syncing

(use "strlib")

(define *base-dir* (tilde-expand "~/.digamma/anaxagoras"))
(define *notes* {})
(define *urls* {})

(define (prompt-string s)
 (display s)
 (read-string))

(define (int-read ) ; read until "^\.$"
 (with r (read-string)
  (cond
   (eq? r ".") '()
   (eq? r #e) '()
   else (cons r (int-read)))))

(define (new-note) 
    (let  ((f (open (format "~s/notes/~a" *base-dir* (sys/time)) :write)) 
          (title (prompt-string "title: "))
          (data (int-read)))
        (display (format "~s\n" title) f)
        (display (string-join data "\n") f)
        (newline f)
        (close f)
        (cset! *notes* title (port-filename f))))

(define (new-url)
    (let ((title (prompt-string "url-title: "))
         (data (prompt-string "url: ")))
        (cset! *urls* title data)))

(define (list-notes)
    (display "in list notes\n")
    (foreach-proc 
        (lambda (k) (display (format "~s\n" k)))
        (keys *notes*)))

(define (list-urls)
    (display "in list-urls\n")
    (foreach-proc 
        (lambda (k)
            (display (format "~s\n" k)))
        (keys *urls*)))

(define (about-anaxagoras)
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

(define (run-shutdown) 
    "dump *notes* and *url* to disk"
    (let ((fnote (open (format "~s/notes.ss" *base-dir*) :write))
          (furls (open (format "~s/urls.ss" *base-dir*) :write)))
     (display "{\n" fnote)
     (foreach-proc
      (lambda (k)
       (display (format "~S ~S\n" k (nth *notes* k)) fnote))
      (keys *notes*))
     (display "}\n" fnote)
     (close fnote)
     (display "{\n" furls)
     (foreach-proc
      (lambda (k)
       (display (format "~S ~S\n" k (nth *urls* k)) furls))
      (keys *urls*))
     (display "}\n" furls)
     (close furls)))

(define (anaxagoras)
 (display "> ")
 (with c (read-string)
  (if
    (eq? (cond ; case macro would be useful here...
            (eq? c "n") (new-note)
            (eq? c "u") (new-url)
            (eq? c "l") (list-notes)
            (eq? c "L") (list-urls)
            (eq? c "h") (about-anaxagoras)
            (eq? c "?") (about-anaxagoras)
            (eq? c "e") (edit-note)
            (eq? c "q") 'QUIT 
            else (display "Invalid command\n"))
        'QUIT)
        (run-shutdown)
        (anaxagoras))))

;; load the notes & urls list
(let ((fnote (open (format "~s/notes.ss" *base-dir*) :read))
      (furls (open (format "~s/urls.ss" *base-dir*) :read)))
    (set! *notes* (read fnote))
    (set! *urls* (read furls))
    (close fnote)
    (close furls))

(display "Welcome to anaxagoras, a note/url organizer\n")
(anaxagoras)
