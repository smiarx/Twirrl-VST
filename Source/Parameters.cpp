#include "Parameters.h"


ParamFloat::ParamFloat (TwirrlAudioProcessor* pr, String pid, String nm, NormalisableRange<float> r, float def, void (TwirrlAudioProcessor::*cb)(float))
   : AudioProcessorParameterWithID (pid, nm), processor(pr), range (r), value (def), defaultValue (def), callback(cb)
{
}

ParamFloat::ParamFloat (TwirrlAudioProcessor* pr, String pid, String nm, float minValue, float maxValue, float def, void (TwirrlAudioProcessor::*cb)(float))
   : AudioProcessorParameterWithID (pid, nm), processor(pr), range (minValue, maxValue), value (def), defaultValue (def), callback(cb)
{
}

ParamFloat::~ParamFloat() {}

float ParamFloat::getValue() const                              { return range.convertTo0to1 (value); }
float ParamFloat::getDefaultValue() const                       { return range.convertTo0to1 (defaultValue); }
int ParamFloat::getNumSteps() const                             { return AudioProcessorParameterWithID::getNumSteps(); }
float ParamFloat::getValueForText (const String& text) const    { return range.convertTo0to1 (text.getFloatValue()); }

String ParamFloat::getText (float v, int length) const
{
    String asText (range.convertFrom0to1 (v), 2);
    return length > 0 ? asText.substring (0, length) : asText;
}

ParamFloat& ParamFloat::operator= (float newValue)
{
    if (value != newValue)
        setValueNotifyingHost (range.convertTo0to1 (newValue));

    return *this;
}



void ParamFloat::setValue (float newValue){
    value = range.convertFrom0to1 (newValue);
    (processor->*callback)(value);
}
