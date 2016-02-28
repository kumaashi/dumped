: A 123 * . ; (test)
: B 2   * A ; (test)
: C 3   * B ; (test)
: D 4   * C ; (test)
: E 5   * D ; (test)
: YASAI
1 - 
dup
if
  foo
  YASAI
else
  bar
  123 . drop 1 E .
then
;

: main 10 YASAI halt ;






