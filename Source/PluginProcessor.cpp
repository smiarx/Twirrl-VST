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
    lutInit();
    addParameter (cutoff = new ParamFloat (this, "cutoff", // parameter ID
                                                  "Cutoff", // parameter name
                                                  0.0f,   // mininum value
                                                  20.0f,   // maximum value
                                                  8.f, &TwirrlAudioProcessor::updateCutoff)); // default value
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
        osc = new Osc(sampleRate, 80);
        vcf = new VCF(sampleRate, 600, 2.3);
}

void TwirrlAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
            osc->setFreq(freq);
        }
        else if (m.isNoteOff())
        {
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
    for (int channel = 0; channel < 1; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        osc->process(channelData, buffer.getNumSamples());
        vcf->process(channelData, buffer.getNumSamples());

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

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TwirrlAudioProcessor();
}
