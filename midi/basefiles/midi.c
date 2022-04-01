#include <stdio.h>

#include "midifile.h"

void read_midi(char* midifile)
{
    MidiFileEvent_t event;
    unsigned char* data;
    MidiFile_t md = MidiFile_load(midifile);

    event = MidiFile_getFirstEvent(md);

    while (event) {
        // A completer :
        // affichage du nom des notes
        // affichage de la durÈe des notes

        event = MidiFileEvent_getNextEventInFile(event);
    }
}

int main(int argc, char** argv)
{
    read_midi(argv[1]);
}
