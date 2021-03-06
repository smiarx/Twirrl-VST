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
#include "Effects.h"





//==============================================================================
/**
*/
        
class ParamFloat;
class ParamBool;
class Voice;


enum ParamID{
    lforateID,
    vibratoID,
    pwmID,
    sawID,
    sqID,
    subID,
    noiseID,
    cutoffID,
    vcflfoID,
    vcfenvID,
    resID,
    aID,
    dID,
    sID,
    rID,
    levelID,

    chorusID,
    crateID,
    phaserID,
    prateID,
    pfeedbackID,
    delayID,
    dtimeID,
    dfeedbackID,
    dwetID
};


class TwirrlAudioProcessor  : public AudioProcessor
{
    LFO lfo;
    Voice* voices;
    Voicer voicer;
    Effects effects;
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
    ParamFloat* pwm;
    ParamFloat* saw;
    ParamFloat* sq;
    ParamFloat* sub;
    ParamFloat* noise;
    ParamFloat* cutoff;
    ParamFloat* vcflfo;
    ParamFloat* vcfenv;
    ParamFloat* res;
    ParamFloat* a;
    ParamFloat* d;
    ParamFloat* s;
    ParamFloat* r;
    ParamFloat* level;

    ParamBool* chorus;
    ParamFloat* crate;
    ParamBool* phaser;
    ParamFloat* prate;
    ParamFloat* pfeedback;
    ParamBool* delay;
    ParamFloat* dtime;
    ParamFloat* dfeedback;
    ParamFloat* dwet;

    void updateParameter(ParamID, float value);
    void updateParameter(ParamID, bool value);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwirrlAudioProcessor)
};




#endif  // PLUGINPROCESSOR_H_INCLUDED
