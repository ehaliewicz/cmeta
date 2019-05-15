The first stage of the compiler, stripping comments

~~~~
; assembler first version of strip_comments (which is raw CMETA assembly)
cat strip_comments.cm | ./casm > strip_comments.cs  

; with strip_comments.cs, we can process any basic cmeta program and remove comments from it
; we'll try this with the second version, strip_comments_with_comments.cm


; first, pass it through strip_comments.cs, and pass the result to casm
cat strip_comments_with_comments.cm | ./cm strip_comments.cs | ./casm > strip_comments_2.cs

; compare the original assembled version with the version which had comments stripped out
; they should be exactly the same
diff strip_comments.cs strip_comments_2.cs ; no difference

~~~~

