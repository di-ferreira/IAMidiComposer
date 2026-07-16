#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.hpp"
#include "ui/PianoRollComponent.hpp"
#include "ui/PresetManager.hpp"

namespace aimidi::plugin {

class AiMidiComposerEditor : public juce::AudioProcessorEditor,
                              public juce::DragAndDropContainer,
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
    juce::TextButton  regenButton_{"Regen Region"};
    juce::TextButton  diffButton_{"Show Diff"};
    juce::Label       statusLabel_;
    juce::Label       seedLabel_;
    juce::Slider      seedSlider_;

    // Presets
    juce::ComboBox   presetSelector_;
    juce::TextButton savePresetButton_{"Save Preset"};
    juce::TextButton deletePresetButton_{"Delete Preset"};
    PresetManager    presetManager_;

    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> seedAttachment_;

    // History
    juce::ComboBox            historyBox_;
    juce::Array<juce::String> promptHistory_;

    // Piano roll
    PianoRollComponent pianoRoll_;

    // State
    juce::ValueTree state_;

    // Diff state caching
    std::vector<PianoRollNote> diffBeforeNotes_;
    bool diffMode_ = false;

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

    // Generation
    void onGenerate();
    void onRegenerate();
    void onDiffToggle();

    // Presets
    void refreshPresetList();
    void onPresetSelected();
    void onSavePreset();
    void onDeletePreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiMidiComposerEditor)
};

} // namespace aimidi::plugin
