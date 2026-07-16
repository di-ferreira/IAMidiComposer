#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.hpp"
#include "ui/PianoRollComponent.hpp"

namespace aimidi::plugin {

class AiMidiComposerEditor : public juce::AudioProcessorEditor,
                              private juce::TextEditor::Listener,
                              private juce::ComboBox::Listener {
public:
    explicit AiMidiComposerEditor(AiMidiComposerProcessor& p);
    ~AiMidiComposerEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AiMidiComposerProcessor& proc_;

    // UI Components
    juce::TextEditor  promptBox_;
    juce::ComboBox    workflowSelector_;
    juce::TextButton  generateButton_{"Generate"};
    juce::Label       statusLabel_;
    juce::Label       seedLabel_;
    juce::Slider      seedSlider_;

    // History
    juce::ComboBox            historyBox_;
    juce::Array<juce::String> promptHistory_;

    // Piano roll
    PianoRollComponent pianoRoll_;

    // State
    juce::ValueTree state_;

    // Listeners
    void textEditorTextChanged(juce::TextEditor&) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void comboBoxChanged(juce::ComboBox*) override;

    // Helpers
    void loadState();
    void saveState();
    void addToHistory(const juce::String& prompt);
    void updateHistoryBox();
    void updateWorkflowList();
    void onGenerate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiMidiComposerEditor)
};

} // namespace aimidi::plugin
