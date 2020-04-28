/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
Copyright (C) 2020 Translational NeuroEngineering Laboratory

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

#include "RangeCheck.h"
#include "RangeCheckEditor.h"

using namespace RangeCheck;

//Change all names for the relevant ones, including "Processor Name"
Node::Node() 
	: GenericProcessor	("Range Check")
	, inputChannel 		(-1)
	, eventChannel 		(0)
	, minVal 			(0)
	, maxVal 			(1)
    , numChannels       (0)
    , activeChannels    ({})
    , currState          (false)
{
    setProcessorType(PROCESSOR_TYPE_FILTER);
}

Node::~Node()
{}

void Node::process(AudioSampleBuffer& buffer)
{
	/*
	If the processor needs to handle events, this method must be called onyl once per process call
	If spike processing is also needing, set the argument to true
	*/
	//checkForEvents(false);
	//int numChannels = getNumOutputs();
    int nSamples = getNumSamples(inputChannel);
    const float* const rp = buffer.getReadPointer(inputChannel);
    juce::int64 startTs = getTimestamp(inputChannel);

    for (int i = 0; i < nSamples; i++)
    {
        // currently false and between values. turn on!
        if (!currState && (rp[i] < maxVal && rp[i] > minVal)) 
        {
            currState = true;
            triggerEvent(startTs, i); 
        }
        // currently true and outside of values. turn off!
        else if (currState && (rp[i] > maxVal || rp[i] < minVal))
        {
            currState = false;
            triggerEventOff(startTs, i);
            
        }
    }
	
}

void Node::triggerEvent(juce::int64 bufferTs, int offset)
{
    // Create events
    MetaDataValueArray mdArray;
    int currEventChan = eventChannel;
    juce::uint8 ttlDataOn = 1 << currEventChan;
    juce::int64 eventTsOn = bufferTs + offset;
    TTLEventPtr eventOn = TTLEvent::createTTLEvent(eventChannelPtr, eventTsOn,
        &ttlDataOn, sizeof(juce::uint8), mdArray, currEventChan);
    addEvent(eventChannelPtr, eventOn, offset);
}

void Node::triggerEventOff(juce::int64 bufferTs, int offset)
{
    // Create off event
    MetaDataValueArray mdArray;
    int currEventChan = eventChannel;
    juce::uint8 ttlDataOff = 0;
    juce::int64 eventTsOff = bufferTs + offset;
    TTLEventPtr eventOff = TTLEvent::createTTLEvent(eventChannelPtr, eventTsOff,
        &ttlDataOff, sizeof(juce::uint8), mdArray, currEventChan);
    addEvent(eventChannelPtr, eventOff, offset);
}

void Node::createEventChannels()
{
    
    // add detection event channel
    const DataChannel* in = getDataChannel(inputChannel);

    if (!in)
    {
        eventChannelPtr = nullptr;
        return;
    }

    float sampleRate = in->getSampleRate();
    EventChannel* chan = new EventChannel(EventChannel::TTL, 8, 1, sampleRate, this);
    chan->setName("Range Check output");
    chan->setDescription("1 when within Range. 0 when outside of range");
    chan->setIdentifier("range.event");

    // metadata storing source data channel

    MetaDataDescriptor sourceChanDesc(MetaDataDescriptor::UINT16, 3, "Source Channel",
        "Index at its source, Source processor ID and Sub Processor index of the channel that triggers this event", "source.channel.identifier.full");
    MetaDataValue sourceChanVal(sourceChanDesc);
    uint16 sourceInfo[3];
    sourceInfo[0] = in->getSourceIndex();
    sourceInfo[1] = in->getSourceNodeID();
    sourceInfo[2] = in->getSubProcessorIdx();
    sourceChanVal.setValue(static_cast<const uint16*>(sourceInfo));
    chan->addMetaData(sourceChanDesc, sourceChanVal);

    // event-related metadata!
    //for (auto desc : eventMetaDataDescriptors)
    //{
    //    chan->addEventMetaData(desc);
    //}

    eventChannelPtr = eventChannelArray.add(chan);
}

void Node::setParameter(int parameterIndex, float newValue) 
{
    switch (parameterIndex)
    {
    case EVENT_CHANNEL:
        eventChannel = newValue;
        break;
    case INPUT_CHANNEL:
        inputChannel = newValue;
        break;
    case MIN_VAL:
        minVal = newValue;
        break;
    case MAX_VAL:
        maxVal = newValue;
        break;
    }
}

void Node::updateSettings()
{
    activeChannels = getActiveInputs();
    numChannels = activeChannels.size();
    editor->update();
}


AudioProcessorEditor* Node::createEditor()
{
    editor = new RangeCheckEditor(this, true);
    return editor;
}

Array<int> Node::getActiveInputs()
{
    int numInputs = getNumInputs();
    
    auto ed = static_cast<RangeCheckEditor*>(getEditor());

    if (numInputs == 0 || !ed)
    {
        return Array<int>();
    }
    //int x = (activeChannels).getUnchecked(289357092356);
    Array<int> activeChannels = ed->getActiveChannels();
    return activeChannels;
}


void Node::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("RANGECHECK");

    // -- Save params -- //
    mainNode->setAttribute("eventChannel", eventChannel);
    mainNode->setAttribute("inputChannel", inputChannel);
    mainNode->setAttribute("minVal", minVal);
    mainNode->setAttribute("maxVal", maxVal);
}

void Node::loadCustomParametersFromXml()
{
    if (parametersAsXml)
    {
        forEachXmlChildElementWithTagName(*parametersAsXml, mainNode, "RANGECHECK")
        {
            // Load other params
            eventChannel = mainNode->getDoubleAttribute("eventChannel");
            inputChannel = mainNode->getDoubleAttribute("inputChannel");
            minVal = mainNode->getDoubleAttribute("minVal");
            maxVal = mainNode->getDoubleAttribute("maxVal");
        }
    }
    editor->update();
}
