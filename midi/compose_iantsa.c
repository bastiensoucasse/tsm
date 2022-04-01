/**
 * ########################################################################
 *
 * PRAISYYYYYYY: <3
 *
 * Je t'ai mis juste l'enregistrement dans un nouveau fichier plutôt
 * que d'écraser l'original, dans le code y'a rien qui a changé à part
 * le nom du fichier de sortie, passé en argument
 *
 * Les seules modifs, je les ai marqué avec /** CHANGED
 * (je me suis permis pcq en vrai c'est trop chiant pour les tests sans,
 * j'espère que tu m'en voudras pas bb,
 * mais sinon j'ai pas touché ton code, promis)
 *
 * Pour lancer après compilation t'as juste à faire :
 * ./compose_iantsa data/starwars1.mid output/starwars1_iantsa.mid
 *
 * LC: :)
 *
 *  ########################################################################
 */

#include <stdio.h>
#include <stdlib.h>

#include "midifile.h"

static int tab[12][12];
static int cpt;

static void
usage(char* progname)
{
    /** CHANGED: Y'a 3 arguments. */
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
    /// unsigned char* data;
    int prev = -1, cur;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        // A completer :
        // affichage du nom des notes
        // affichage de la durÈe des notes

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
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++)
            sum += tab[i][j];

        for (int j = 0; j < 12; j++)
            tab[i][j] /= sum;
    }

    // Display intervals tab
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 12; j++)
            printf("%d ", tab[i][j]);
        printf("\n");
    }
    printf("N: %d\n", cpt);
}

/** CHANGED: On a un fichier de destination. */
static void
compose(char* midifile, char* savefile)
{
    MidiFile_t md = MidiFile_load(midifile);
    int prev = -1, cur = rand() % 12;
    float prob[12];
    float random;

    MidiFileEvent_t event = MidiFile_getFirstEvent(md);
    while (event) {
        if (MidiFileEvent_isNoteStartEvent(event) && MidiFileNoteStartEvent_getChannel(event) != 10) {
            // if (prev > 0)
            // {
            //     for (int i = 0; i < 12; i++)
            //         prob[i] = tab[prev][i] + tab[prev][i-1];

            //     random = (float) rand() / RAND_MAX;
            //     for (int i = 0; i < 12; i++)
            //         if (tab[prev][i] > 0 && random > prob[i])
            //         {
            //             cur = i;
            //             break;
            //         }
            // }

            MidiFileNoteStartEvent_setNote(event, 50);

            // prev = cur;
        }

        event = MidiFileEvent_getNextEventInFile(event);
    }

    // Display intervals tab
    // for (int i = 0; i < 12; i++)
    // {
    //     for (int j = 0; j < 12; j++)
    //         printf("%d ", tab[i][j]);
    //     printf("\n");
    // }
    // printf("N: %d\n", cpt);

    /** CHANGED: On save dans le fichier de sortie. */
    MidiFile_save(md, savefile);
}

int main(int argc, char** argv)
{
    /** CHANGED: Y'a 3 arguments (voir usage). */
    if (argc != 3)
        usage(argv[0]);

    read_midi(argv[1]);

    /** CHANGED: On passe le 3ème argument. */
    compose(argv[1], argv[2]);

    return EXIT_SUCCESS;
}
