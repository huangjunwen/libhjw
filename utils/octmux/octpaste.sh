#!/bin/sh

# make temp file
TMPBUF=`mktemp /tmp/tmpbuf.XXXXXX` 
trap "rm $TMPBUF" EXIT 

case $1 in
    python)
        # line discipline for python REPL
        #   Filter out all blank lines and comment lines (grep)
        #   Then add blank line before all top level structure (tr,sed)
        grep . | grep -v '^[[:space:]]#' | tr '\n' '\f' | sed 's/\f\([^ tab]\)/\f\f\1/g' | tr '\f' '\n' > $TMPBUF 
        ;;
    *)
        # line discipline for other
        cat > $TMPBUF
        ;;
esac

# Append one more blank line to EOF (echo)
echo >> $TMPBUF

# Paste data to the next pane
tmux loadb $TMPBUF \; pasteb -d -t .+1
