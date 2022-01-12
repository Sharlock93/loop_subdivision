let &path.="header,src,"
nnoremap <A-m> :AsyncRun build.bat<cr>
nnoremap <A-p> :AsyncStop<cr>
nnoremap <A-r> :AsyncRun run.bat<cr>
nnoremap <F7> :cn<cr>

nnoremap gd :YcmCompleter GoTo<CR>

autocmd BufNewFile,BufRead *.sh_it set ft=sh_it
autocmd BufNewFile,BufRead *.sh_it set commentstring=//\ %s

" set errorformat=\.\.%f|%l| %m
