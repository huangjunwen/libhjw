" what it is:
"   send the selected text (under visual mode) to an external REPL to eval
" install:
"   you need to install tmux first, and config tmux to have multiple panes
"   with pane 1 the REPL (eg, python)
"   then
"       cp * ~/.vim/plugin/
" usage:
"   select text ('v', 'V' or 'C-V') then 'e'
" TODO:
"   * auto detect tmux's existense
"   * auto detect editing content and pass different arg to octpaste
"   * not hard code to pane 1

vmap e y:call system("~/.vim/plugin/octpaste.sh python", getreg('"'))<CR><CR>
