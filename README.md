# Ankee
A tool to turn japanese subtitles into anki flashcards easily using the JMDict dictionary

![Ankee](https://user-images.githubusercontent.com/59704655/233614419-b593fb0f-7727-4281-8cf9-a21a5efbfcb8.png)

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
[Armee](github.com/gitRaiku/Armee) is a tool that does the same thing but for german subtitles
