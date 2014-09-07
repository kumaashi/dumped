\ http://stackoverflow.com/questions/18246756/what-is-the-most-common-filename-extension-of-a-forth-source-code-file

: TEST                      \ test                    
  dup sin . CR              \ n1 sin[n1] print        
  drop                      \ n1                      
  0.1 +                     \ n1+0.1                  
  dup 1.0  > if ret then    \ n1+0.1  b[n1+0.1 > 1.0] 
  drop                      \ n1+0.1                  
  test                      \ test                    
;

:
  main 0 TEST
;

