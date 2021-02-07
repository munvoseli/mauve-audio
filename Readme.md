Use this to build the audio and play.

    ./caudio p; ffplay -f f32le -ar 44100 -ac 1 p.raw

# The notation

`p` changes the pitch - takes one base 12 argument, in which 0 is A, and b is G#. The pitch will move to the nearest A, B#, E, F, Gb. If there are six half-steps between the current pitch and the specified pitch, the pitch will move to the pitch above.

`po` will move the pitch up or down an octave - takes `+` or `-`.

`al` and `rl` specify attack length and release length - the larger the attack length, the more gradual the onset.

`v` takes a float between 0 and 1, and sets the volume.

`w` will cause a note to play. Its argument is the length of the note.