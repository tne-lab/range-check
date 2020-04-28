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


#ifndef RANGECHECK_EDITOR_H_INCLUDED
#define RANGECHECK_EDITOR_H_INCLUDED

#include "RangeCheck.h"
#include <EditorHeaders.h>

namespace RangeCheck
{
    class RangeCheckEditor
        : public GenericEditor
        , public ComboBox::Listener
        , public Label::Listener
        , public Button::Listener
    {

    public:
        RangeCheckEditor(Node* n, bool useDefaultParameterEditors);
        ~RangeCheckEditor();
        
        void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
        void labelTextChanged(Label* labelThatHasChanged) override; // looks for those
        void buttonClicked(Button* buttonClick) override;
        // Override means builtin. We are just changing its function, but it will get called automatically when needed
        void channelChanged(int chan, bool newState) override;

        void updateSettings() override;

        void startAcquisition() override;
        void stopAcquisition() override;

        //Visualizer* createNewCanvas() override {}; // creates canvas


    private:
        Node* processor;
        
        ScopedPointer<Label> inputChannelLabel;
        ScopedPointer<Label> outputChannelLabel;
        ScopedPointer<ComboBox> inputBox;
        ScopedPointer<ComboBox> outputBox;

        ScopedPointer<Label> minLabel;
        ScopedPointer<Label> maxLabel;
        ScopedPointer<Label> minLabelE;
        ScopedPointer<Label> maxLabelE;
        ScopedPointer<ToggleButton> minNeg;
        ScopedPointer<ToggleButton> maxNeg;
        
        Label* createLabel(const String& name, const String& text,
            juce::Rectangle<int> bounds);

        Label* createEditable(const String& name, const String& initialValue,
            const String& tooltip, juce::Rectangle<int> bounds);

        bool updateFloatLabel(Label* label, float min, float max,
            float defaultValue, float* out);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangeCheckEditor);
    };

}


#endif // RangeCheck_EDITOR_H_INCLUDED
