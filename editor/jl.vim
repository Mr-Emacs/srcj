autocmd BufRead,BufNewFile *.jl set filetype=jl

if exists("b:current_syntax")
      finish
endif

syntax match   jlComment      "//.*$"
syntax region  jlBlockComment start="/\*" end="\*/"

syntax region  jlString       start=+"+ end=+"+ skip=+\\"+ contains=jlStringEscape
syntax match   jlStringEscape "\\\." contained

syntax match   jlDirective    "#\(import\|load\)\>"

syntax keyword jlKeyword
      \ if else while return true false
      \ open extern import load as comptime

syntax keyword jlType number bool string void

syntax match   jlNumber       "\<[0-9]\+\>"

syntax match   jlBuiltin      "|<<\||>>"
syntax match   jlBuiltin      "->"

syntax match   jlFn          "\<fn\>"         nextgroup=jlFuncDecl  skipwhite skipnl
syntax match   jlStruct      "\<struct\>"      nextgroup=jlTypeName  skipwhite skipnl
syntax match   jlInterface   "\<interface\>"   nextgroup=jlTypeName  skipwhite skipnl
syntax match   jlModule      "\<module\>"      nextgroup=jlTypeName  skipwhite skipnl
syntax match   jlEnum        "\<enum\>"        nextgroup=jlEnumName  skipwhite skipnl

syntax match   jlFuncDecl    "[a-zA-Z_][a-zA-Z0-9_]*\ze\s*("  contained
syntax match   jlTypeName    "[A-Z][a-zA-Z0-9_]*"              contained
syntax match   jlEnumName    "[A-Z][a-zA-Z0-9_]*"              contained

syntax match   jlUserType    "\<[A-Z][a-zA-Z0-9_]*\>"

syntax match   jlPtrType     "\*\+\zs[a-zA-Z_][a-zA-Z0-9_]*\>"

syntax match   jlEnumVariant "^\s*\zs[A-Z][a-zA-Z0-9_]*\ze\s*[,={]"

syntax match   jlTypeNS   "\([A-Z][a-zA-Z0-9_]*\)\ze::[a-zA-Z_][a-zA-Z0-9_]*"
syntax match   jlFuncNS   "[A-Z][a-zA-Z0-9_]*::\zs[a-zA-Z_][a-zA-Z0-9_]*"
syntax match   jlConstNS  "\([a-z][a-zA-Z0-9_]*\)\ze::[a-zA-Z_][a-zA-Z0-9_]*"
syntax match   jlFuncNSLo "[a-z][a-zA-Z0-9_]*::\zs[a-zA-Z_][a-zA-Z0-9_]*"

syntax match   jlImportAlias
      \ "#import\s\+\"[^\"]\+\"\s\+as\s\+\zs[a-zA-Z_][a-zA-Z0-9_]*"

syntax match   jlVarDecl    "[a-zA-Z_][a-zA-Z0-9_]*\ze\s*:="
syntax match   jlVarAnnot "[a-zA-Z_][a-zA-Z0-9_]*\ze\s*:\s*\(\*\+\|\[.\{-}\]\)\?[a-zA-Z_][a-zA-Z0-9_]*"
syntax match   jlTypeAnnot  "[a-zA-Z_][a-zA-Z0-9_]*\s*:\s*\(\*\+\|\[.\{-}\]\)\?\zs[a-zA-Z_][a-zA-Z0-9_]*\>"
syntax match   jlArrayType  "\[.\{-}\]\zs[a-zA-Z_][a-zA-Z0-9_]*\>"


highlight default link jlArrayType Type
highlight default link jlComment        Comment
highlight default link jlBlockComment   Comment
highlight default link jlString         String
highlight default link jlStringEscape   SpecialChar
highlight default link jlDirective      PreProc
highlight default link jlKeyword        Keyword
highlight default link jlFn             Keyword
highlight default link jlStruct         Keyword
highlight default link jlInterface      Keyword
highlight default link jlModule         Keyword
highlight default link jlEnum           Keyword
highlight default link jlType           Type
highlight default link jlNumber         Constant
highlight default link jlBuiltin        Operator
highlight default link jlFuncDecl       Function
highlight default link jlTypeName       Type
highlight default link jlEnumName       Type
highlight default link jlUserType       Type
highlight default link jlPtrType        Type
highlight default link jlEnumVariant    Constant
highlight default link jlTypeNS         Type
highlight default link jlFuncNS         Function
highlight default link jlConstNS        Constant
highlight default link jlFuncNSLo       Function
highlight default link jlImportAlias    Constant
highlight default link jlVarDecl        Identifier
highlight default link jlVarAnnot       Identifier
highlight default link jlTypeAnnot      Type

let b:current_syntax = "jl"
