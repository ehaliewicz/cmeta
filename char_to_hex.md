The second stage of the compiler, converts character constants to hex values
e.g. `T '"'` becomes `T 0x22`

~~~~

; compile char_to_hex (which has comments) by passing it through strip_comments.cs
; and passing the result to casm
cat char_to_hex.cm | ./cm strip_comments.cs | ./casm > char_to_hex.cs 


; with char_to_hex.cs, we can compile the second version, char_to_hex_with_chars.cm
; which has character constants in it

cat char_to_hex_with_chars.cm | ./cm strip_comments | \
    			        ./cm char_to_hex.cs | \
                                ./casm > char_to_hex_2.cs

; compare the original assembled version with the new version
; they should be exactly the same
diff char_to_hex.cs char_to_hex_2.cs
~~~~ 
