#include <stdio.h>
#include <stdlib.h>

#include "midifile.h"

static char* notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <file>.\n", progname);
    exit(EXIT_FAILURE);
}

static void
read_midi(char* midifile)
{
    MidiFile_t md = MidiFile_load(midifile);

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        long time = MidiFileEvent_getTick(event);
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteEndEvent_getChannel(event) != 10) {
            printf("\nNote at %ld:\n", time);

            // Note
            char* note = notes[MidiFileNoteStartEvent_getNote(event) % 12];
            printf("  - Note: %s.\n", note);

            // Velocity
            int velocity = MidiFileNoteStartEvent_getVelocity(event);
            printf("  - Velocity: %d.\n", velocity);

            // Duration
            int duration = MidiFileEvent_getTick(MidiFileNoteStartEvent_getNoteEndEvent(event)) - MidiFileEvent_getTick(event);
            printf("  - Duration: %d.\n", duration);

            // Channel
            int channel = MidiFileNoteStartEvent_getChannel(event);
            printf("  - Channel: %d.\n", channel);
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }
}

static void
transpose(char* input_filename, char* output_filename, int transpose)
{
    MidiFile_t input = MidiFile_load(input_filename);
}

int main(int argc, char** argv)
{
    if (argc != 4)
        usage(argv[0]);

    transpose(argv[1], argv[2], atoi(argv[3]));

    return EXIT_SUCCESS;
}
