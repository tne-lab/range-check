/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
Copyright (C) 2020 Translational NeuroEngineering Laboratory, MGH

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "RangeCheckEditor.h"
//#include "RangeCheck.h"
#include <string>  // stoi, stof
#include <climits> // INT_MAX
#include <cfloat>  // FLT_MAX

using namespace RangeCheck;

RangeCheckEditor::RangeCheckEditor(Node* parentNode, bool useDefaultParameterEditors = true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 220;
    //tabText = "Range Check";
    processor = static_cast<Node*>(parentNode);

    const int TEXT_HT = 18;
    /* ------------- Top row (channels) ------------- */
    int xPos = 12;
    int yPos = 36;

    inputChannelLabel = createLabel("InputChanL", "In:", { xPos, yPos, 30, TEXT_HT });
    addAndMakeVisible(inputChannelLabel);

    inputBox = new ComboBox("Input channel");
    inputBox->setTooltip("Continuous channel to analyze");
    inputBox->setBounds(xPos += 33, yPos, 40, TEXT_HT);
    inputBox->addListener(this);
    addAndMakeVisible(inputBox);

    outputChannelLabel = createLabel("OutL", "Out:", { xPos += 50, yPos, 40, TEXT_HT });
    addAndMakeVisible(outputChannelLabel);

    outputBox = new ComboBox("Out event channel");
    for (int chan = 1; chan <= 8; chan++)
        outputBox->addItem(String(chan), chan);
    outputBox->setSelectedId(processor->eventChannel + 1);
    outputBox->setBounds(xPos += 45, yPos, 40, TEXT_HT);
    outputBox->setTooltip("Output event channel");
    outputBox->addListener(this);
    addAndMakeVisible(outputBox);

    xPos = 12;
    yPos = 70;

    minLabel = createLabel("minL", "Min Value:", { xPos, yPos, 80, TEXT_HT });
    addAndMakeVisible(minLabel);

    minLabelE = createEditable("minLE", "0", "Minimum Value to Accept", {xPos += 90, yPos, 40, TEXT_HT} );
    addAndMakeVisible(minLabelE);
    juce::Rectangle<int> bounds;
    minNeg = new ToggleButton("* -1");
    minNeg->setBounds(bounds = { xPos+=50, yPos, 40, TEXT_HT });
    minNeg->addListener(this);
    minNeg->setTooltip("Make this number negative");
    //instantButton->setColour(ToggleButton::textColourId, Colours::white);
    addAndMakeVisible(minNeg);

    xPos = 12;
    maxLabel = createLabel("maxL", "Max Value:", {  xPos, yPos += 30, 80, TEXT_HT });
    addAndMakeVisible(maxLabel);

    maxLabelE = createEditable("maxLE", "1", "Maximum Value to Accept", {xPos += 90, yPos, 40, TEXT_HT} );
    addAndMakeVisible(maxLabelE);

    maxNeg = new ToggleButton("* -1");
    maxNeg->setBounds(bounds = { xPos += 50, yPos, 40, TEXT_HT });
    maxNeg->addListener(this);
    maxNeg->setTooltip("Make this number negative");
    //instantButton->setColour(ToggleButton::textColourId, Colours::white);
    addAndMakeVisible(maxNeg);
}

RangeCheckEditor::~RangeCheckEditor() {}

void RangeCheckEditor::updateSettings()
{
    // update input combo box
    int numInputs = processor->numChannels;
    int currInputId = inputBox->getSelectedId();
    int currInputChan = processor->inputChannel;

    if (numInputs == 0)
    {
        if (currInputId != 0)
        {
            inputBox->clear(sendNotificationSync);
        }
        // else do nothing - box is already empty.
    }
    else
    {
        Array<int> activeChannels = processor->activeChannels;
        inputBox->clear(dontSendNotification);
        for (int chan = 1; chan <= numInputs; ++chan)
        {
            // using 1-based ids since 0 is reserved for "nothing selected"
            inputBox->addItem(String(activeChannels[chan-1]+1), chan);
            //if (currInputId == chan)
            if (activeChannels[chan - 1] == currInputChan)
            {
                inputBox->setSelectedId(chan, dontSendNotification);
            }
        }

        if (inputBox->getSelectedId() == 0)
        {
            // default to first channel
            inputBox->setSelectedId(1, sendNotificationSync);
        }
        if (processor->inputChannel == -1 && numInputs > 0)
        {
            processor->setParameter(Node::INPUT_CHANNEL,
                static_cast<float>(activeChannels[0]));
        }
    }
    
    // Update if processor has different info than editor (most likely only when loading in xml file)
    minLabelE->setText(String(abs(processor->minVal)), dontSendNotification);
    maxLabelE->setText(String(abs(processor->maxVal)), dontSendNotification);
    if (processor->minVal < 0)
    {
        minNeg->setToggleState(true, dontSendNotification);
    }
    if (processor->maxVal < 0)
    {
        maxNeg->setToggleState(true, dontSendNotification);
    }
    outputBox->setSelectedId(processor->eventChannel + 1);
}

void RangeCheckEditor::channelChanged(int chan, bool newState)
{
    if (!acquisitionIsActive)
    {
        processor->updateSettings();
    }
}

void RangeCheckEditor::startAcquisition()
{
    outputBox->setEnabled(false);
}

void RangeCheckEditor::stopAcquisition()
{
    outputBox->setEnabled(true);
}

void RangeCheckEditor::buttonClicked(Button* buttonClicked)
{
    if (buttonClicked == minNeg)
    {
        processor->minVal *= -1;
    }
    if (buttonClicked == maxNeg)
    {
        processor->maxVal *= -1;
    }
}

void RangeCheckEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == inputBox)
    {
        Array<int> activeChannels = processor->activeChannels;
        processor->setParameter(Node::INPUT_CHANNEL,
            static_cast<float>(activeChannels[inputBox->getSelectedId()-1] - 1));
    }

    else if (comboBoxThatHasChanged == outputBox)
    {
        processor->setParameter(Node::EVENT_CHANNEL,
            static_cast<float>(outputBox->getSelectedId() - 1));
    }
}

void RangeCheckEditor::labelTextChanged(Label* labelThatHasChanged)
{
    auto processor = static_cast<Node*>(getProcessor());
    if (labelThatHasChanged == minLabelE)
    {
        float newVal;
        if (updateFloatLabel(labelThatHasChanged, 0, FLT_MAX, 0.0, &newVal))
        {
            if (minNeg->getToggleState())
            {
                newVal *= -1;
            }
            processor->setParameter(Node::MIN_VAL, static_cast<float>(newVal));
        }
    }
    if (labelThatHasChanged == maxLabelE)
    {
        float newVal;
        if (updateFloatLabel(labelThatHasChanged, 0, FLT_MAX, 1.0, &newVal))
        {
            if (maxNeg->getToggleState())
            {
                newVal *= -1;
            }
            processor->setParameter(Node::MAX_VAL, static_cast<float>(newVal));
        }
    }
}


Label* RangeCheckEditor::createLabel(const String& name, const String& text,
    juce::Rectangle<int> bounds)
{
    Label* label = new Label(name, text);
    label->setBounds(bounds);
    label->setFont(Font("Small Text", 12, Font::plain));
    label->setColour(Label::textColourId, Colours::darkgrey);
    return label;
}

Label* RangeCheckEditor::createEditable(const String& name, const String& initialValue,
    const String& tooltip, juce::Rectangle<int> bounds)
{
    Label* editable = new Label(name, initialValue);
    editable->setEditable(true);
    editable->addListener(this);
    editable->setBounds(bounds);
    editable->setColour(Label::backgroundColourId, Colours::grey);
    editable->setColour(Label::textColourId, Colours::white);
    if (tooltip.length() > 0)
    {
        editable->setTooltip(tooltip);
    }
    return editable;
}


bool RangeCheckEditor::updateFloatLabel(Label* label, float min, float max,
    float defaultValue, float* out)
{
    const String& in = label->getText();
    float parsedFloat;
    try
    {
        parsedFloat = std::stof(in.toRawUTF8());
    }
    catch (const std::logic_error&)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    *out = jmax(min, jmin(max, parsedFloat));

    label->setText(String(*out), dontSendNotification);
}