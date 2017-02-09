#include "Parameters.h"
#include "PluginProcessor.h"


ParamFloat::ParamFloat (TwirrlAudioProcessor& pr, ParamID nb, String pid, String nm, NormalisableRange<float> r, float def)
   : AudioProcessorParameterWithID (pid, nm), numID(nb), processor(pr), range (r), value (def), defaultValue (def)
{
}

ParamFloat::ParamFloat (TwirrlAudioProcessor& pr, ParamID nb, String pid, String nm, float minValue, float maxValue, float def)
   : AudioProcessorParameterWithID (pid, nm), numID(nb), processor(pr), range (minValue, maxValue), value (def), defaultValue (def)
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
    processor.updateParameter(numID, value);
}





//==============================================================================
ParamBool::ParamBool (TwirrlAudioProcessor& pr, ParamID nb, String pid, String nm, bool def)
   : AudioProcessorParameterWithID (pid, nm),
     numID(nb),
     processor(pr),
     value (def ? 1.0f : 0.0f),
     defaultValue (value)
{
}

ParamBool::~ParamBool() {}

float ParamBool::getValue() const                               { return value; }
float ParamBool::getDefaultValue() const                        { return defaultValue; }
int ParamBool::getNumSteps() const                              { return 2; }
float ParamBool::getValueForText (const String& text) const     { return text.getIntValue() != 0 ? 1.0f : 0.0f; }
String ParamBool::getText (float v, int /*length*/) const       { return String ((int) (v > 0.5f ? 1 : 0)); }

ParamBool& ParamBool::operator= (bool newValue)
{
    if (get() != newValue)
        setValueNotifyingHost (newValue ? 1.0f : 0.0f);

    return *this;
}


void ParamBool::setValue (float newValue){
    value = newValue;
    processor.updateParameter(numID, value>=0.5f);
}
