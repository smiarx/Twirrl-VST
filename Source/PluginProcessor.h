/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "LFO.h"
#include "Voicer.h"





//==============================================================================
/**
*/
        
class ParamFloat;
class Voice;


enum ParamID{
    lforateID,
    vibratoID,
    sawID,
    sqID,
    noiseID,
    cutoffID,
    vcflfoID,
    vcfenvID,
    resID,
    aID,
    dID,
    sID,
    rID
};


class TwirrlAudioProcessor  : public AudioProcessor
{
    LFO lfo;
    Voice* voices;
    Voicer voicer;
    bool running;

public:
    //==============================================================================
    TwirrlAudioProcessor();
    ~TwirrlAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;



    void  doVoice(void (Voice::*func)(float), float value);
    ParamFloat* lforate;
    ParamFloat* vibrato;
    ParamFloat* saw;
    ParamFloat* sq;
    ParamFloat* noise;
    ParamFloat* cutoff;
    ParamFloat* vcflfo;
    ParamFloat* vcfenv;
    ParamFloat* res;
    ParamFloat* a;
    ParamFloat* d;
    ParamFloat* s;
    ParamFloat* r;

    void updateParameter(ParamID, float value);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwirrlAudioProcessor)
};




#endif  // PLUGINPROCESSOR_H_INCLUDED
