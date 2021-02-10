The purpose of Mauve is to provide a command line and text editor substitute for graphical music applications - it is a language for making music.

Use `make` and `make play` to compile the program, build the example audio, and play.

# The notation

`p` changes the pitch - takes one base 12 argument, in which 0 is A, and b is G#. The pitch will move to the nearest A, B#, E, F, Gb. If there are six half-steps between the current pitch and the specified pitch, the pitch will move to the pitch above.

`po` will move the pitch up or down an octave - takes `+` or `-`.

`al` and `rl` specify attack length and release length - the larger the attack length, the more gradual the onset.

`v` takes a float between 0 and 1, and sets the volume.

`w` will cause a note to play. Its argument is the length of the note.

`defmacro [name]` - macros are defined at the end of the string. The name must not contain whitespace. They are preprocessed (just string replacement) with `usemacro [name]`.