Use this to build the audio and play.

    ./caudio p; ffplay -f f32le -ar 44100 -ac 1 p.raw

# The notation

`p` changes the pitch - takes one base 12 argument, in which 0 is A, and b is G#.