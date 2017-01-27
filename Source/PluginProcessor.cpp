/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <math.h>
#include <iostream>

#include "Voice.h"
#include "Parameters.h"

//==============================================================================
TwirrlAudioProcessor::TwirrlAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    running=false;
    lutInit();
    addParameter (cutoff = new ParamFloat (*this, cutoffID, "cutoff", "Cutoff",  0.0f, 20.0f, 8.f));
    addParameter (res = new ParamFloat (*this, resID, "res", "Resonnance", 0.0f, 4.0f, 2.f));
    addParameter (a = new ParamFloat (*this, aID, "attack", "Attack", 0.01f, 5.0f, 0.01f));
    addParameter (d = new ParamFloat (*this, dID, "decay", "Decay", 0.01f, 5.0f, 0.5f));
    addParameter (s = new ParamFloat (*this, sID, "sustain", "Sustain", 0.f, 1.f, 0.5f));
    addParameter (r = new ParamFloat (*this, rID, "release", "Release", 0.01f, 10.0f, 1.5f));
}

TwirrlAudioProcessor::~TwirrlAudioProcessor()
{
}

//==============================================================================
const String TwirrlAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TwirrlAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TwirrlAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double TwirrlAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TwirrlAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TwirrlAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TwirrlAudioProcessor::setCurrentProgram (int index)
{
}

const String TwirrlAudioProcessor::getProgramName (int index)
{
    return String();
}

void TwirrlAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TwirrlAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voices = new Voice(*this, sampleRate, samplesPerBlock);
    running=true;
}

void TwirrlAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    if(running){
            delete voices;
            running=false;
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
    bool TwirrlAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
    {
      #if JucePlugin_IsMidiEffect
        ignoreUnused (layouts);
        return true;
      #else
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
            return false;

        // This checks if the input layout matches the output layout
       #if ! JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
       #endif

        return true;
      #endif
    }
#endif

void TwirrlAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{


    buffer.clear();

    int time;
    MidiMessage m;
 
    for (MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    {
        if (m.isNoteOn())
        {
            double freq=pow(2, (m.getNoteNumber()-69.)/12.)*440.;
            std::cout << freq;
            std::cout << "\n";
            voices->start(freq);
        }
        else if (m.isNoteOff())
        {
            voices->release();
        }
        else if (m.isAftertouch())
        {
        }
        else if (m.isPitchWheel())
        {
        }
 
    }


    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    Voice* vc = voices;
    for (int channel = 0; channel < 1; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        for(int i=0; i<NVOICES; ++i)
            if(vc->isRunning())
                (vc++)->process(channelData, buffer.getNumSamples());

    }
}

//==============================================================================
bool TwirrlAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* TwirrlAudioProcessor::createEditor()
{
    return new TwirrlAudioProcessorEditor (*this);
}

//==============================================================================
void TwirrlAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TwirrlAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


void TwirrlAudioProcessor::updateParameter(ParamID id, float value){
    if(!running)
        return;
    switch(id){
        default:
            Voice* vc = voices;
            for(int i=0; i<NVOICES; i++)
                (vc++)->updateParameter(id, value);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwirrlAudioProcessor();
}
