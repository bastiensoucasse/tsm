#include <stdio.h>
#include <stdlib.h>

#include "midifile.h"

static const char* const notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static MidiFile_t md;

static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <input> <output> <transpose_value>.\n", progname);
    exit(EXIT_FAILURE);
}

static void
read_midi()
{
    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        long time = MidiFileEvent_getTick(event);
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteEndEvent_getChannel(event) != 10) {
            printf("\nNote at %ld:\n", time);

            // Note
            const char* const note = notes[MidiFileNoteStartEvent_getNote(event) % 12];
            printf("  - Note: %s.\n", note);

            // Velocity
            const int velocity = MidiFileNoteStartEvent_getVelocity(event);
            printf("  - Velocity: %d.\n", velocity);

            // Duration
            const int duration = MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event)) - MidiFileEvent_getTick(event);
            printf("  - Duration: %d.\n", duration);

            // Channel
            const int channel = MidiFileNoteStartEvent_getChannel(event);
            printf("  - Channel: %d.\n", channel);
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }
}

static void
transpose(const int transpose_value)
{
    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteEndEvent_getChannel(event) != 10) {
            const int note = MidiFileNoteStartEvent_getNote(event);
            MidiFileNoteStartEvent_setNote(event, note + transpose_value);
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }
}

int main(int argc, char** argv)
{
    if (argc != 4)
        usage(argv[0]);
    
    md = MidiFile_load(argv[1]);

    // read_midi(argv[1]);

    transpose(atoi(argv[3]));

    MidiFile_save(md, argv[2]);
    return EXIT_SUCCESS;
}
