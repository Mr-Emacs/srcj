;;; jl-mode.el --- Major mode for the JL language -*- lexical-binding: t -*-

(defvar jl-mode-hook nil)

(defvar jl-mode-map
  (let ((map (make-keymap)))
    map)
  "Keymap for JL major mode.")

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.jl\\'" . jl-mode))

;; Keywords
(defconst jl-keywords
  '("fn" "if" "else" "while" "return" "true" "false"
    "struct" "open" "extern" "interface" "import" "module" "load" "as"
    "comptime"))

;; Intrinsic directives (#import, #load)
(defconst jl-directives
  '("#import" "#load"))

;; Types
(defconst jl-types
  '("number" "bool" "string" "void"))

(defconst jl-font-lock-keywords
  (list
   ;; Directives: #import, #load
   `(,(regexp-opt jl-directives 'symbols) . font-lock-preprocessor-face)

   ;; extern string e.g. extern "raylib"
   `("extern\\s-+\"\\([^\"]+\\)\"" (1 font-lock-string-face))

   ;; Keywords
   `(,(regexp-opt jl-keywords 'symbols) . font-lock-keyword-face)

   ;; Types
   `(,(regexp-opt jl-types 'symbols) . font-lock-type-face)

   ;; Function declarations: fn name(
   `("\\bfn\\s-+\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\s-*(" (1 font-lock-function-name-face))

   ;; Module definition: module Raylib
   `("\\bmodule\\s-+\\([A-Z][a-zA-Z0-9_]*\\)" (1 font-lock-type-face))

   ;; Struct names: struct Foo
   `("\\bstruct\\s-+\\([A-Z][a-zA-Z0-9_]*\\)" (1 font-lock-type-face))

   ;; Interface names: interface Foo
   `("\\binterface\\s-+\\([A-Z][a-zA-Z0-9_]*\\)" (1 font-lock-type-face))

   ;; #import "Raylib" as alias — alias in constant face (distinct from type face)
   `("#import\\s-+\"[^\"]+\"\\s-+as\\s-+\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
     (1 font-lock-constant-face))

   ;; Capitalized namespace: Raylib::something
   `("\\([A-Z][a-zA-Z0-9_]*\\)::\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
     (1 font-lock-type-face)
     (2 font-lock-function-name-face))

   ;; Lowercase alias call: raylib::init_window — alias in constant face
   `("\\([a-z][a-zA-Z0-9_]*\\)::\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
     (1 font-lock-constant-face)
     (2 font-lock-function-name-face))

   ;; Variable declarations: name :=
   `("\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\s-*:=" (1 font-lock-variable-name-face))

   ;; Type annotations: name: type
   `("\\([a-zA-Z_][a-zA-Z0-9_]*\\)\\s-*:\\s-*\\([a-zA-Z_][a-zA-Z0-9_]*\\)"
     (1 font-lock-variable-name-face)
     (2 font-lock-type-face))

   ;; Numbers
   `("\\b[0-9]+\\b" . font-lock-constant-face)

   ;; Rotation operators
   `("|<<\\||>>" . font-lock-builtin-face)

   ;; Return type arrow ->
   `("->" . font-lock-builtin-face)))

;; Syntax table — teach Emacs about comments and strings
(defvar jl-mode-syntax-table
  (let ((st (make-syntax-table)))
    ;; // line comments
    (modify-syntax-entry ?/ ". 124b" st)
    (modify-syntax-entry ?* ". 23" st)
    (modify-syntax-entry ?\n "> b" st)
    ;; strings
    (modify-syntax-entry ?\" "\"" st)
    ;; _ is a word character
    (modify-syntax-entry ?_ "w" st)
    st)
  "Syntax table for `jl-mode'.")

;; Indentation — 4 spaces, no tabs
(defun jl-indent-line ()
  "Indent current line for JL mode."
  (interactive)
  (let ((indent (jl--calculate-indent)))
    (indent-line-to indent)))

(defun jl--calculate-indent ()
  "Calculate the indentation for the current line."
  (save-excursion
    (beginning-of-line)
    (let ((cur-line (point))
          (indent 0))
      (save-excursion
        (condition-case nil
            (progn
              (backward-up-list)
              (setq indent (+ (current-indentation) 4)))
          (error (setq indent 0))))
      ;; Closing brace — dedent
      (when (looking-at "\\s-*}")
        (setq indent (max 0 (- indent 4))))
      indent)))

;;;###autoload
(define-derived-mode jl-mode prog-mode "JL"
  "Major mode for editing JL source files."
  :syntax-table jl-mode-syntax-table
  (setq-local font-lock-defaults '(jl-font-lock-keywords))
  (setq-local comment-start "// ")
  (setq-local comment-end "")
  (setq-local comment-start-skip "//+\\s-*")
  (setq-local indent-line-function #'jl-indent-line)
  (setq-local tab-width 4)
  (setq-local indent-tabs-mode nil))

(provide 'jl-mode)
;;; jl-mode.el ends here
