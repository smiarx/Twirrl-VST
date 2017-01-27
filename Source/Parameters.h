#ifndef _Parameters_H
#define _Parameters_H


#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


class ParamFloat  : public AudioProcessorParameterWithID
{
public:
    /** Creates a ParamFloat with an ID, name, and range.
        On creation, its value is set to the default value.
    */
    ParamFloat (TwirrlAudioProcessor& pr, ParamID nb, String parameterID, String name,
                         NormalisableRange<float> normalisableRange,
                         float defaultValue);

    /** Creates a ParamFloat with an ID, name, and range.
        On creation, its value is set to the default value.
        For control over skew factors, you can use the other
        constructor and provide a NormalisableRange.
    */
    ParamFloat (TwirrlAudioProcessor& pr, ParamID nb, String parameterID, String name,
                         float minValue,
                         float maxValue,
                         float defaultValue);

    /** Destructor. */
    ~ParamFloat();

    /** Returns the parameter's current value. */
    float get() const noexcept                  { return value; }
    /** Returns the parameter's current value. */
    operator float() const noexcept             { return value; }

    /** Changes the parameter's current value. */
    ParamFloat& operator= (float newValue);

    /** Provides access to the parameter's range. */
    NormalisableRange<float> range;


private:
    //==============================================================================
    float value, defaultValue;
    ParamID numID;

    TwirrlAudioProcessor& processor;

    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    String getText (float, int) const override;
    float getValueForText (const String&) const override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamFloat)
};


#endif
