The first stage of the compiler, stripping comments

~~~~
cat strip_comments.cm | ./cmc > strip_comments.cs  ; compile first stage strip_comments program

; with strip_comments.cs, we can process any basic cmeta program and remove comments from it 

cat strip_comments_with_comments.cm | ./cm strip_comments.cs > strip_comments_2.cs

diff strip_comments.cs strip_comments_2.cs ; no difference

~~~~
