#include <stdio.h>
#include <stdlib.h>

#include "midifile.h"

static float tab[12][12];
static int cpt;

static void
usage(char* progname)
{
    fprintf(stderr, "Usage: %s <input> <output>.\n", progname);
    exit(EXIT_FAILURE);
}

static void
read_midi(char* midifile)
{
    // Initialize intervals tab
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++)
            tab[i][j] = 0;

    MidiFile_t md = MidiFile_load(midifile);
    int prev = -1, cur;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteStartEvent_getChannel(event) != 10) {
            // Pitch
            cur = MidiFileNoteStartEvent_getNote(event);
            if (prev > 0) {
                tab[prev % 12][cur % 12]++;
                cpt++;
            }

            prev = cur;
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }

    // Transform to probabilities
    int sum;
    for (int i = 0; i < 12; i++) {
        sum = 0;
        for (int j = 0; j < 12; j++)
            sum += tab[i][j];

        if (sum > 0)
            for (int j = 0; j < 12; j++)
                tab[i][j] /= sum;
        
    }

    // Display intervals tab
    // for (int i = 0; i < 12; i++) {
    //     for (int j = 0; j < 12; j++)
    //         printf("%f ", tab[i][j]);
    //     printf("\n");
    // }
    // printf("N: %d\n", cpt);
}

static void
compose(char* midifile, char* savefile)
{
    MidiFile_t md = MidiFile_load(midifile);
    int prev = -1, cur;
    float prob[12];
    float random;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteStartEvent_getChannel(event) != 10) {
            if (prev < 0)
                cur = MidiFileNoteStartEvent_getNote(event)%12;

            else
            {
                prob[0] = tab[prev][0];
                for (int i = 1; i < 12; i++)
                    prob[i] = tab[prev][i] + prob[i-1];

                // for (int i = 0; i < 12; i++)
                //     printf("%f ", prob[i]);
                // printf("\n");

                random = (float) rand() / RAND_MAX;
                for (int i = 0; i < 12; i++)
                    if (random < prob[i])
                    {
                        cur = i;
                        break;
                    }
            }

            MidiFileNoteStartEvent_setNote(event, 6*12 + cur);
            printf("%d ", 6*12 + cur);

            prev = cur;
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }

    // Display intervals tab
    // for (int i = 0; i < 12; i++)
    // {
    //     for (int j = 0; j < 12; j++)
    //         printf("%f ", tab[i][j]);
    //     printf("\n");
    // }
    // printf("N: %d\n", cpt);

    MidiFile_save(md, savefile);
}

int main(int argc, char** argv)
{
    if (argc != 3)
        usage(argv[0]);

    read_midi(argv[1]);

    compose(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
