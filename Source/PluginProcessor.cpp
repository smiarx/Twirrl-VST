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
#include <stdlib.h>

#include "Voice.h"
#include "Parameters.h"

//==============================================================================
TwirrlAudioProcessor::TwirrlAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
     voices(nullptr),
     voicer(Voicer(&voices)),
     effects(Effects(*this))
{
    srand(time(NULL));
    running=false;
    lutInit();
    addParameter (lforate = new ParamFloat (*this, lforateID, "lforate", "LFO rate",  0.0f, 10.f, 0.8f));
    addParameter (vibrato = new ParamFloat (*this, vibratoID, "vibrato", "Vibrato",  0.0f, 1.f, 0.2f));
    addParameter (saw = new ParamFloat (*this, sawID, "saw", "Saw Level",  0.0f, 1.f, 1.f));
    addParameter (sq = new ParamFloat (*this, sqID, "square", "Pulse Level",  0.0f, 1.f, 1.f));
    addParameter (sub = new ParamFloat (*this, subID, "sub", "Sub Level",  0.0f, 1.f, 0.5f));
    addParameter (noise = new ParamFloat (*this, noiseID, "noise", "Noise Level",  0.0f, 1.f, 0.1));
    addParameter (cutoff = new ParamFloat (*this, cutoffID, "cutoff", "Cutoff",  -12.0f, 60.0f, 12.f));
    addParameter (res = new ParamFloat (*this, resID, "res", "Resonnance", 0.0f, 4.0f, 2.f));
    addParameter (vcflfo = new ParamFloat (*this, vcflfoID, "vcflfo", "VCF LFO Modulation",  0.f, 30.0f, 0.f));
    addParameter (vcfenv = new ParamFloat (*this, vcfenvID, "vcfenv", "VCF Envelope Modulation",  -30.f, 30.0f, 0.f));
    addParameter (a = new ParamFloat (*this, aID, "attack", "Attack", 0.01f, 5.0f, 0.01f));
    addParameter (d = new ParamFloat (*this, dID, "decay", "Decay", 0.01f, 5.0f, 0.5f));
    addParameter (s = new ParamFloat (*this, sID, "sustain", "Sustain", 0.f, 1.f, 0.5f));
    addParameter (r = new ParamFloat (*this, rID, "release", "Release", 0.01f, 10.0f, 1.5f));

    addParameter (chorus = new ParamBool (*this, chorusID, "chorus", "Chorus", false));
}

TwirrlAudioProcessor::~TwirrlAudioProcessor()
{
    if(running)
        delete voices;
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
    lfo.start(sampleRate, samplesPerBlock, lforate->get());
    effects.start(sampleRate, samplesPerBlock);
    voices = static_cast<Voice*> (operator new(sizeof(Voice)*NVOICES));
    Voice *vc=voices;
    for(int i=0; i<NVOICES; ++i)
        new(vc++) Voice(*this, sampleRate, samplesPerBlock, lfo.getBuf());
    //vc = voices;
    //for(int i=0; i<NVOICES; ++i)
    //    std::cout << &(vc++)->running << "\n";

    running=true;
}

void TwirrlAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    if(running){
            delete voices;
            effects.stop();

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
            //double freq=pow(2, (m.getNoteNumber()-69.)/12.)*440.;
            //std::cout << freq;
            //std::cout << "\n";
            voicer.noteOn(m.getNoteNumber(),100);
        }
        else if (m.isNoteOff())
        {
            voicer.noteOff(m.getNoteNumber());
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
    
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);
    int numSamples=buffer.getNumSamples();

    Voice* vc = voices;
    lfo.process();

    //run voices
    for(int i=0; i<NVOICES; ++i){
        if(vc->isRunning())
            vc->process(left, numSamples);
        vc++;
    }

    //copy left to right
    float *lft=left, *rght=right;
    for(int i=0; i<numSamples; ++i)
        *(rght++) = *(lft++);

    effects.process(left,right,numSamples);
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





void inline TwirrlAudioProcessor::doVoice(void (Voice::*func)(float), float value){
    Voice* vc=voices;
    for (int i=0; i<NVOICES; ++i)
        ((vc++)->*func)(value);
}


void TwirrlAudioProcessor::updateParameter(ParamID id, float value){
    if(!running)
        return;
    switch(id){
        case lforateID:
            lfo.updateRate(value);
            break;
        case vibratoID:
            doVoice(&Voice::updateVibrato,value);
            break;
        case sawID:
            doVoice(&Voice::updateSaw,value);
            break;
        case sqID:
            doVoice(&Voice::updateSq,value);
            break;
        case subID:
            doVoice(&Voice::updateSub,value);
            break;
        case noiseID:
            doVoice(&Voice::updateNoise,value);
            break;
        case cutoffID:
            doVoice(&Voice::updateCutoff,value);
            break;
        case vcflfoID:
            doVoice(&Voice::updateVCFLFO,value);
            break;
        case vcfenvID:
            doVoice(&Voice::updateVCFEnv,value);
            break;
        case resID:
            doVoice(&Voice::updateRes,value);
            break;
        case aID:
            doVoice(&Voice::updateAttack,value);
            break;
        case dID:
            doVoice(&Voice::updateDecay,value);
            break;
        case sID:
            doVoice(&Voice::updateSustain,value);
            break;
        case rID:
            doVoice(&Voice::updateRelease,value);
            break;
    }
}


void TwirrlAudioProcessor::updateParameter(ParamID id, bool value){
    if(!running)
        return;
    switch(id){
        case chorusID:
            effects.setChorus(value);
            break;
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwirrlAudioProcessor();
}
