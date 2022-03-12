#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ladspa.h"

#define VOCAL_REMOVE_INPUT1 0 // Input left
#define VOCAL_REMOVE_INPUT2 1 // Input right
#define VOCAL_REMOVE_OUTPUT 2 // Mono output

LADSPA_Descriptor* g_psDescriptor;

typedef struct {
    LADSPA_Data* m_pfInputBuffer1;
    LADSPA_Data* m_pfInputBuffer2;
    LADSPA_Data* m_pfOutputBuffer;
} VocalRemove;

LADSPA_Handle
instantiateVocalRemove(const LADSPA_Descriptor* Descriptor, unsigned long SampleRate)
{
    return malloc(sizeof(VocalRemove));
}

void connectPortToVocalRemove(LADSPA_Handle Instance, unsigned long Port, LADSPA_Data* DataLocation)
{
    switch (Port) {
    case VOCAL_REMOVE_INPUT1:
        ((VocalRemove*)Instance)->m_pfInputBuffer1 = DataLocation;
        break;
    case VOCAL_REMOVE_INPUT2:
        ((VocalRemove*)Instance)->m_pfInputBuffer2 = DataLocation;
        break;
    case VOCAL_REMOVE_OUTPUT:
        ((VocalRemove*)Instance)->m_pfOutputBuffer = DataLocation;
        break;
    }
}

void runVocalRemove(LADSPA_Handle Instance, unsigned long SampleCount)
{
    VocalRemove* psVocalRemove;
    LADSPA_Data* pfInput1;
    LADSPA_Data* pfInput2;
    LADSPA_Data* pfOutput;
    unsigned long lSampleIndex;

    psVocalRemove = (VocalRemove*)Instance;
    pfOutput = psVocalRemove->m_pfOutputBuffer;
    pfInput1 = psVocalRemove->m_pfInputBuffer1;
    pfInput2 = psVocalRemove->m_pfInputBuffer2;

    for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++)
        pfOutput[lSampleIndex] = pfInput1[lSampleIndex] - pfInput2[lSampleIndex];
}

void cleanupVocalRemove(LADSPA_Handle Instance)
{
    free(Instance);
}

void _init()
{
    char** pcPortNames;
    LADSPA_PortDescriptor* piPortDescriptors;
    LADSPA_PortRangeHint* psPortRangeHints;

    g_psDescriptor = (LADSPA_Descriptor*)malloc(sizeof(LADSPA_Descriptor));
    if (!g_psDescriptor)
        return;

    g_psDescriptor->UniqueID = 1910;
    g_psDescriptor->Label = strdup("vocalremover");
    g_psDescriptor->Properties = LADSPA_PROPERTY_REALTIME;
    g_psDescriptor->Name = strdup("Sound VocalRemove");
    g_psDescriptor->Maker = strdup("Master Enseignements");
    g_psDescriptor->Copyright = strdup("None");
    g_psDescriptor->PortCount = 3;
    piPortDescriptors = (LADSPA_PortDescriptor*)calloc(3, sizeof(LADSPA_PortDescriptor));
    g_psDescriptor->PortDescriptors = (const LADSPA_PortDescriptor*)piPortDescriptors;
    piPortDescriptors[VOCAL_REMOVE_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[VOCAL_REMOVE_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[VOCAL_REMOVE_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    pcPortNames = (char**)calloc(3, sizeof(char*));
    g_psDescriptor->PortNames = (const char**)pcPortNames;
    pcPortNames[VOCAL_REMOVE_INPUT1] = strdup("Input1");
    pcPortNames[VOCAL_REMOVE_INPUT2] = strdup("Input2");
    pcPortNames[VOCAL_REMOVE_OUTPUT] = strdup("Output");
    psPortRangeHints = ((LADSPA_PortRangeHint*)calloc(3, sizeof(LADSPA_PortRangeHint)));
    g_psDescriptor->PortRangeHints = (const LADSPA_PortRangeHint*)psPortRangeHints;
    psPortRangeHints[VOCAL_REMOVE_INPUT1].HintDescriptor = 0;
    psPortRangeHints[VOCAL_REMOVE_INPUT2].HintDescriptor = 0;
    psPortRangeHints[VOCAL_REMOVE_OUTPUT].HintDescriptor = 0;
    g_psDescriptor->instantiate = instantiateVocalRemove;
    g_psDescriptor->connect_port = connectPortToVocalRemove;
    g_psDescriptor->activate = NULL;
    g_psDescriptor->run = runVocalRemove;
    g_psDescriptor->run_adding = NULL;
    g_psDescriptor->deactivate = NULL;
    g_psDescriptor->cleanup = cleanupVocalRemove;
}

void _fini()
{
    long lIndex;
    if (g_psDescriptor) {
        free((char*)g_psDescriptor->Label);
        free((char*)g_psDescriptor->Name);
        free((char*)g_psDescriptor->Maker);
        free((char*)g_psDescriptor->Copyright);
        free((LADSPA_PortDescriptor*)g_psDescriptor->PortDescriptors);
        for (lIndex = 0; lIndex < g_psDescriptor->PortCount; lIndex++)
            free((char*)(g_psDescriptor->PortNames[lIndex]));
        free((char**)g_psDescriptor->PortNames);
        free((LADSPA_PortRangeHint*)g_psDescriptor->PortRangeHints);
        free(g_psDescriptor);
    }
}

const LADSPA_Descriptor*
ladspa_descriptor(unsigned long Index)
{
    if (Index == 0)
        return g_psDescriptor;
    return NULL;
}
