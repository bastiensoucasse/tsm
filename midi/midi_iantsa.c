#include <stdio.h>
#include <stdlib.h>

#include "midifile.h"


static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <file>.\n", progname);
    exit(EXIT_FAILURE);
}

static char*
getNameNote(int p)
{
    char* notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return notes[p%12];


    // char base = 'C';

    // int n = p % 12;

    // if (n < 5)
    // {
    //     res[0] = base + (n/2);
    //     if (n % 2 != 0) res[1] = '#';
    // }
    
    // res[0] = base + n;
    // if (n % 2 != 0) res[1] = '#';
    // return res;

}

static void
read_midi(char* midifile)
{
    MidiFile_t md = MidiFile_load(midifile);
    /// unsigned char* data;
    int p, d;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        // A completer :
        // affichage du nom des notes
        // affichage de la durÈe des notes

        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteStartEvent_getChannel(event) != 10)
        {
            // Pitch
            p = MidiFileNoteStartEvent_getNote(event);
            printf("\nNote played: %s\n", getNameNote(p));

            // Duration
            d = MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event)) - MidiFileEvent_getTick(event);
            printf("Duration: %d\n", d);

            // Velocity
            printf("Velocity: %d\n", MidiFileNoteStartEvent_getVelocity(event));
        }
        
        event = MidiFileEvent_getNextEventInFile(event);
    }
}

static void
transpose(char* midifile, char* savefile, int t)
{
    MidiFile_t md = MidiFile_load(midifile);
    
    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) { 
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteStartEvent_getChannel(event) != 10)
            MidiFileNoteStartEvent_setNote(event, MidiFileNoteStartEvent_getNote(event) + t);
        
        event = MidiFileEvent_getNextEventInFile(event);
    }
    

    MidiFile_save(md, savefile);
}

int main(int argc, char** argv)
{
    if (argc < 2)
        usage(argv[0]);

    read_midi(argv[1]);

    transpose(argv[1], argv[2], atoi(argv[3]));

    return EXIT_SUCCESS;
}
