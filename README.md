# Ankee
A tool to turn japanese subtitles into anki flashcards easily using the JMDict dictionary which uses ncurses for the tui.

![Ankee](https://github.com/gitRaiku/ankee/blob/master/resources/Ankee.png?raw=true)

# Installation
Running 
```
sudo make install
```
should build it and copy the client (ankeec) and server (ankeed) to ``/usr/local/bin/``

Adding 
```
if [ "$ANKEEC" = "1" ]'
        exec ankeec "$(cat /tmp/ankeect)" "$(cat /tmp/ankeecp)"'
end
```
to your ``config.fish`` or the equivalent for your shell, and modifying the ``sankee`` script for your terminal+shell combo should let the ``resources/new_sub_to_anki.lua`` mpv script work.

# Usage
You can start the server by just running ``ankeed``

Now you can use the tool by running ``ankeec <display text> [path/to/audio/file]``

Or by using the ``resources/new_sub_to_anki.lua`` mpv script which takes the current mpv subtitle and sends that to ``ankeec``

# Warning
``ankeed`` may take up a quite a bit of memory while idling and especially searching specific words since it written in python and i haven't gotten to optimise it that much...

# Armee
[Armee](https://github.com/gitRaiku/ankee) is a tool that does the same thing but for german subtitles
