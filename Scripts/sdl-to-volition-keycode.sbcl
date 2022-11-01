(defun get-input ()
  (let ((str (make-array '(0)
                         :element-type 'base-char
                         :fill-pointer 0
                         :adjustable t)))
    (with-output-to-string (str-stream str)
      (loop
       (let ((c (read-char t nil nil)))
         (if (null c)
             (return)
             (write-char c str-stream)))))
    str))

(defun is-separator? (c)
  (or (char= c #\ ) (char= c #\Linefeed) (char= c #\Return) (char= c #\Tab)))

(defun get-tokens (str start)
  (if (null start)
      nil
      (let ((p1 (position-if #'(lambda (c)
                                 (not (is-separator? c)))
                             str :start start)))
        (if p1
            (let ((p2 (position-if #'is-separator? str :start (+ p1 1))))
              (if p2
                  (cons (subseq str p1 p2)
                        (get-tokens str
                                    (let ((p3 (position #\Linefeed str :start (+ p2 1))))
                                      (if p3
                                          (+ p3 1)
                                          nil))))
                  nil))
            nil))))

(defun convert-sdl-to-volition-keycode (str)
  (let ((res (make-array '(0)
                         :element-type 'base-char
                         :fill-pointer 0
                         :adjustable t))
        (toks (get-tokens str 0)))
    (with-output-to-string (stream res)
      (dolist (tok toks)
        (let ((vol-key (nstring-capitalize
                        (subseq tok
                                (+ (position #\_ tok) 1)))))
          (format stream "~A = ~A,~%" vol-key tok))))
    res))

(let ((res (convert-sdl-to-volition-keycode (get-input))))
  (format t "~A" res))
