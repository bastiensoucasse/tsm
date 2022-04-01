#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midifile.h"

static const char* const notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static MidiFile_t md;
static double matrix[12][12];
static int count;

static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <input> <output>.\n", progname);
    exit(EXIT_FAILURE);
}

static void
fill_matrix()
{
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++)
            matrix[i][j] = 0.;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    int previous_note = -1, note = -1;
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteEndEvent_getChannel(event) != 10) {
            previous_note = note;
            note = MidiFileNoteStartEvent_getNote(event);

            if (previous_note < 0 || note < 0)
                continue;

            matrix[previous_note % 12][note % 12]++;
            count++;
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }

    double sum;
    for (int i = 0; i < 12; i++) {
        sum = 0.; // ERROR: Sum must be reset to zero for each line.

        for (int j = 0; j < 12; j++)
            sum += matrix[i][j];

        if (sum == 0)
            continue;

        for (int j = 0; j < 12; j++)
            matrix[i][j] /= sum; // ERROR: Division must include a floating number.
    }
}

static void
print_matrix()
{
    printf("Matrix (%d notes):\n", count);
    printf("  ");
    for (int j = 0; j < 12; j++)
        printf("  %-2s ", notes[j]);
    printf("\n");
    for (int i = 0; i < 12; i++) {
        printf("%-2s", notes[i]);
        for (int j = 0; j < 12; j++)
            printf(" %.2lf", matrix[i][j]);
        printf("\n");
    }
}

static void
compose()
{
    printf("Composition:\n");

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    int previous_note = -1, note = -1, new_note = -1;
    double probabilities[12], random;
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteEndEvent_getChannel(event) != 10) {
            previous_note = note;
            note = MidiFileNoteStartEvent_getNote(event);

            if (previous_note < 0 || note < 0)
                continue;

            memcpy(probabilities, matrix[previous_note % 12], 12 * sizeof(double));
            for (int j = 1; j < 12; j++)
                probabilities[j] += probabilities[j - 1]; // ERROR: Probabilities aren't correctly computed.

            random = (double)rand() / RAND_MAX;

            for (int j = 0; j < 12; j++) {
                if (probabilities[j] < random)
                    continue;

                new_note = 6 * 12 + j;
                break;
            }

            MidiFileNoteStartEvent_setNote(event, new_note);
            printf("%-2s -> %s\n", notes[note % 12], notes[new_note % 12]);
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
        usage(argv[0]);

    md = MidiFile_load(argv[1]);

    fill_matrix();
    print_matrix();

    printf("\n");

    compose();

    MidiFile_save(md, argv[2]);
    return EXIT_SUCCESS;
}
