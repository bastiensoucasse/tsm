#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ladspa.h"

#define PLUGIN_INPUT1 0
#define PLUGIN_INPUT2 1
#define PLUGIN_OUTPUT1 2
#define PLUGIN_OUTPUT2 3
#define PLUGIN_PARAM 4

typedef struct {
    LADSPA_Data* m_pfInputBuffer1;
    LADSPA_Data* m_pfInputBuffer2;
    LADSPA_Data* m_pfOutputBuffer1;
    LADSPA_Data* m_pfOutputBuffer2;
    LADSPA_Data* m_pfParam;
} Plugin;

LADSPA_Descriptor* g_psDescriptor;
bool noise_gate = true;

LADSPA_Handle
instantiatePlugin(const LADSPA_Descriptor* Descriptor, unsigned long SampleRate)
{
    return malloc(sizeof(Plugin));
}

void connectPortToPlugin(LADSPA_Handle Instance, unsigned long Port, LADSPA_Data* DataLocation)
{
    switch (Port) {
    case PLUGIN_INPUT1:
        ((Plugin*)Instance)->m_pfInputBuffer1 = DataLocation;
        break;
    case PLUGIN_INPUT2:
        ((Plugin*)Instance)->m_pfInputBuffer2 = DataLocation;
        break;
    case PLUGIN_OUTPUT1:
        ((Plugin*)Instance)->m_pfOutputBuffer1 = DataLocation;
        break;
    case PLUGIN_OUTPUT2:
        ((Plugin*)Instance)->m_pfOutputBuffer2 = DataLocation;
        break;
    case PLUGIN_PARAM:
        ((Plugin*)Instance)->m_pfParam = DataLocation;
        break;
    }
}

void runPlugin(LADSPA_Handle Instance, unsigned long SampleCount)
{
    Plugin* psPlugin;
    LADSPA_Data* pfInput1;
    LADSPA_Data* pfInput2;
    LADSPA_Data* pfOutput1;
    LADSPA_Data* pfOutput2;
    LADSPA_Data* pfParam;
    unsigned long i;

    psPlugin = (Plugin*)Instance;
    pfOutput1 = psPlugin->m_pfOutputBuffer1;
    pfOutput2 = psPlugin->m_pfOutputBuffer2;
    pfInput1 = psPlugin->m_pfInputBuffer1;
    pfInput2 = psPlugin->m_pfInputBuffer2;
    pfParam = psPlugin->m_pfParam;

    float energy = 0.;
    for (i = 0; i < SampleCount; i++)
        energy += (pow(pfInput1[i], 2.) + pow(pfInput2[i], 2.)) / (2. * SampleCount);
    
    if (energy >= *(pfParam) && noise_gate)
        noise_gate = false;

    for (i = 0; i < SampleCount; i++)
        if (noise_gate)
            pfOutput1[i] = 0, pfOutput2[i] = 0;
        else
            pfOutput1[i] = pfInput1[i], pfOutput2[i] = pfInput2[i];
}

void cleanupPlugin(LADSPA_Handle Instance)
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
    g_psDescriptor->Label = strdup("plugin");
    g_psDescriptor->Properties = LADSPA_PROPERTY_REALTIME;
    g_psDescriptor->Name = strdup("Sound Plugin");
    g_psDescriptor->Maker = strdup("Master UB");
    g_psDescriptor->Copyright = strdup("None");
    g_psDescriptor->PortCount = 5;
    piPortDescriptors = (LADSPA_PortDescriptor*)calloc(5, sizeof(LADSPA_PortDescriptor));
    g_psDescriptor->PortDescriptors = (const LADSPA_PortDescriptor*)piPortDescriptors;
    piPortDescriptors[PLUGIN_INPUT1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[PLUGIN_INPUT2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[PLUGIN_OUTPUT1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[PLUGIN_OUTPUT2] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    piPortDescriptors[PLUGIN_PARAM] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    pcPortNames = (char**)calloc(5, sizeof(char*));
    g_psDescriptor->PortNames = (const char**)pcPortNames;
    pcPortNames[PLUGIN_INPUT1] = strdup("Input1");
    pcPortNames[PLUGIN_INPUT2] = strdup("Input2");
    pcPortNames[PLUGIN_OUTPUT1] = strdup("Output1");
    pcPortNames[PLUGIN_OUTPUT2] = strdup("Output2");
    pcPortNames[PLUGIN_PARAM] = strdup("Control");
    psPortRangeHints = ((LADSPA_PortRangeHint*)calloc(5, sizeof(LADSPA_PortRangeHint)));
    g_psDescriptor->PortRangeHints = (const LADSPA_PortRangeHint*)psPortRangeHints;
    psPortRangeHints[PLUGIN_INPUT1].HintDescriptor = 0;
    psPortRangeHints[PLUGIN_INPUT2].HintDescriptor = 0;
    psPortRangeHints[PLUGIN_OUTPUT1].HintDescriptor = 0;
    psPortRangeHints[PLUGIN_OUTPUT2].HintDescriptor = 0;
    psPortRangeHints[PLUGIN_PARAM].HintDescriptor = (LADSPA_HINT_BOUNDED_BELOW);
    psPortRangeHints[PLUGIN_PARAM].LowerBound = 0;
    g_psDescriptor->instantiate = instantiatePlugin;
    g_psDescriptor->connect_port = connectPortToPlugin;
    g_psDescriptor->activate = NULL;
    g_psDescriptor->run = runPlugin;
    g_psDescriptor->run_adding = NULL;
    g_psDescriptor->deactivate = NULL;
    g_psDescriptor->cleanup = cleanupPlugin;
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
