#include "PluginEditor.hpp"

namespace aimidi::plugin {

static const auto bgColor       = juce::Colour::fromRGB(28, 28, 32);
static const auto fgColor       = juce::Colour::fromRGB(220, 220, 220);
static const auto accentColor   = juce::Colour::fromRGB(0, 180, 255);
static const auto dimColor      = juce::Colour::fromRGB(120, 120, 120);
static const auto warnColor     = juce::Colour::fromRGB(255, 180, 0);

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
AiMidiComposerEditor::AiMidiComposerEditor(AiMidiComposerProcessor& p)
    : juce::AudioProcessorEditor(&p), proc_(p) {
    setSize(640, 480);

    // ---- Prompt box -------------------------------------------------------
    promptBox_.setMultiLine(true, false);
    promptBox_.setReturnKeyStartsNewLine(false);
    promptBox_.setTabKeyUsedAsCharacter(true);
    promptBox_.setScrollbarsShown(true);
    promptBox_.setFont(juce::Font(16.0f));
    promptBox_.setTextToShowWhenEmpty(
        "Describe your composition... (e.g. 'upbeat pop in C major')", dimColor);
    promptBox_.setColour(juce::TextEditor::backgroundColourId,
                         juce::Colour::fromRGB(24, 24, 28));
    promptBox_.setColour(juce::TextEditor::textColourId, fgColor);
    promptBox_.setColour(juce::TextEditor::outlineColourId,
                         juce::Colour::fromRGB(48, 48, 52));
    promptBox_.setColour(juce::TextEditor::focusedOutlineColourId, accentColor);
    addAndMakeVisible(promptBox_);

    // ---- History dropdown --------------------------------------------------
    historyBox_.setTextWhenNothingSelected("History");
    historyBox_.setColour(juce::ComboBox::backgroundColourId,
                          juce::Colour::fromRGB(24, 24, 28));
    historyBox_.setColour(juce::ComboBox::textColourId, fgColor);
    historyBox_.setColour(juce::ComboBox::outlineColourId,
                          juce::Colour::fromRGB(48, 48, 52));
    historyBox_.setColour(juce::ComboBox::buttonColourId, accentColor);
    addAndMakeVisible(historyBox_);

    // ---- Workflow selector -------------------------------------------------
    updateWorkflowList();
    workflowSelector_.setColour(juce::ComboBox::backgroundColourId,
                                juce::Colour::fromRGB(24, 24, 28));
    workflowSelector_.setColour(juce::ComboBox::textColourId, fgColor);
    workflowSelector_.setColour(juce::ComboBox::outlineColourId,
                                juce::Colour::fromRGB(48, 48, 52));
    workflowSelector_.setColour(juce::ComboBox::buttonColourId, accentColor);
    addAndMakeVisible(workflowSelector_);

    // ---- Seed slider -------------------------------------------------------
    seedSlider_.setRange(0.0, 999999.0, 1.0);
    seedSlider_.setValue(42.0, juce::dontSendNotification);
    seedSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    seedSlider_.setColour(juce::Slider::backgroundColourId,
                          juce::Colour::fromRGB(48, 48, 52));
    seedSlider_.setColour(juce::Slider::thumbColourId, accentColor);
    seedSlider_.setColour(juce::Slider::textBoxTextColourId, fgColor);
    seedSlider_.setColour(juce::Slider::textBoxBackgroundColourId,
                          juce::Colour::fromRGB(24, 24, 28));
    seedSlider_.setColour(juce::Slider::textBoxOutlineColourId,
                          juce::Colour::fromRGB(48, 48, 52));
    seedLabel_.setText("Seed:", juce::dontSendNotification);
    seedLabel_.setColour(juce::Label::textColourId, fgColor);
    addAndMakeVisible(seedSlider_);
    addAndMakeVisible(seedLabel_);

    // ---- Generate button ---------------------------------------------------
    generateButton_.setColour(juce::TextButton::buttonColourId, accentColor);
    generateButton_.setColour(juce::TextButton::textColourOffId,
                              juce::Colour::fromRGB(255, 255, 255));
    generateButton_.setColour(juce::TextButton::buttonOnColourId,
                              accentColor.darker(0.4f));
    generateButton_.setEnabled(false);
    addAndMakeVisible(generateButton_);

    // ---- Regen Region button -----------------------------------------------
    regenButton_.setColour(juce::TextButton::buttonColourId, warnColor);
    regenButton_.setColour(juce::TextButton::textColourOffId,
                           juce::Colour::fromRGB(255, 255, 255));
    regenButton_.setEnabled(false);
    addAndMakeVisible(regenButton_);

    // ---- Diff button -------------------------------------------------------
    diffButton_.setColour(juce::TextButton::buttonColourId,
                          juce::Colour::fromRGB(80, 80, 90));
    diffButton_.setColour(juce::TextButton::textColourOffId,
                          juce::Colour::fromRGB(200, 200, 200));
    diffButton_.setEnabled(false);
    addAndMakeVisible(diffButton_);

    // ---- Status label ------------------------------------------------------
    statusLabel_.setText("Ready", juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId, dimColor);
    addAndMakeVisible(statusLabel_);

    // ---- Piano roll --------------------------------------------------------
    addAndMakeVisible(pianoRoll_);

    // ---- Listeners ---------------------------------------------------------
    promptBox_.addListener(this);
    historyBox_.addListener(this);
    workflowSelector_.addListener(this);

    generateButton_.onClick = [this] { onGenerate(); };
    regenButton_.onClick    = [this] { onRegenerate(); };
    diffButton_.onClick     = [this] { onDiffToggle(); };
    seedSlider_.onValueChange = [this] { saveState(); };

    // Selection callback - enable/disable regen button
    pianoRoll_.onSelectionChanged = [this] {
        const bool canRegen = pianoRoll_.hasSelection()
                           && !promptBox_.getText().trim().isEmpty();
        regenButton_.setEnabled(canRegen);
    };

    // ---- Restore persisted state -------------------------------------------
    loadState();
}

AiMidiComposerEditor::~AiMidiComposerEditor() {
    promptBox_.removeListener(this);
    historyBox_.removeListener(this);
    workflowSelector_.removeListener(this);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::paint(juce::Graphics& g) {
    g.fillAll(bgColor);

    g.setColour(fgColor);
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("AI MIDI Composer", 10, 10, 300, 30,
               juce::Justification::centredLeft);
}

// ---------------------------------------------------------------------------
// Resized
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::resized() {
    auto area = getLocalBounds().reduced(10);

    // Top bar: title is drawn in paint; seed slider on the right
    auto topBar = area.removeFromTop(30);
    auto seedArea = topBar.removeFromRight(250);
    seedLabel_.setBounds(seedArea.removeFromLeft(50));
    seedSlider_.setBounds(seedArea);

    // Prompt box (≈4 lines)
    area.removeFromTop(10);
    auto promptArea = area.removeFromTop(100);
    promptBox_.setBounds(promptArea);

    // History + Workflow row
    area.removeFromTop(10);
    auto controlsRow = area.removeFromTop(30);
    historyBox_.setBounds(controlsRow.removeFromLeft(250));
    controlsRow.removeFromLeft(10);
    workflowSelector_.setBounds(controlsRow);

    // Button row: Generate + Regen + Diff + Status
    area.removeFromTop(10);
    auto bottomRow = area.removeFromTop(40);
    generateButton_.setBounds(bottomRow.removeFromLeft(120));
    bottomRow.removeFromLeft(8);
    regenButton_.setBounds(bottomRow.removeFromLeft(120));
    bottomRow.removeFromLeft(8);
    diffButton_.setBounds(bottomRow.removeFromLeft(100));
    bottomRow.removeFromLeft(8);
    statusLabel_.setBounds(bottomRow);

    // Piano roll takes the remaining space
    area.removeFromTop(10);
    pianoRoll_.setBounds(area);
}

// ---------------------------------------------------------------------------
// TextEditor::Listener
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::textEditorTextChanged(juce::TextEditor&) {
    const bool hasText = !promptBox_.getText().trim().isEmpty();
    generateButton_.setEnabled(hasText);
    regenButton_.setEnabled(hasText && pianoRoll_.hasSelection());
}

void AiMidiComposerEditor::textEditorReturnKeyPressed(juce::TextEditor&) {
    if (juce::ModifierKeys::currentModifiers.isShiftDown()) {
        promptBox_.insertTextAtCaret("\n");
    } else if (!promptBox_.getText().trim().isEmpty()) {
        onGenerate();
    }
}

// ---------------------------------------------------------------------------
// ComboBox::Listener
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::comboBoxChanged(juce::ComboBox* box) {
    if (box == &historyBox_) {
        auto idx = historyBox_.getSelectedItemIndex();
        if (idx >= 0 && idx < promptHistory_.size()) {
            promptBox_.setText(promptHistory_[idx], true);
        }
    } else if (box == &workflowSelector_) {
        saveState();
    }
}

// ---------------------------------------------------------------------------
// Generate
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::onGenerate() {
    auto prompt = promptBox_.getText().trim();
    if (prompt.isEmpty())
        return;

    statusLabel_.setText("Generating...", juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId, accentColor);
    generateButton_.setEnabled(false);

    addToHistory(prompt);
    saveState();

    // TODO: dispatch to the ACE engine via the bridge.
    // For now we simply set the status back to indicate completion.
    statusLabel_.setText("Done", juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId, dimColor);
    generateButton_.setEnabled(true);

    // Clear any prior diff state after a new generation
    diffBeforeNotes_.clear();
    diffMode_ = false;
    diffButton_.setButtonText("Show Diff");
    diffButton_.setEnabled(false);
}

// ---------------------------------------------------------------------------
// Regenerate Region (W5 - Smart Regeneration)
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::onRegenerate() {
    if (!pianoRoll_.hasSelection()) {
        statusLabel_.setText("Select a region first", juce::dontSendNotification);
        return;
    }

    auto prompt = promptBox_.getText().trim();
    if (prompt.isEmpty()) {
        statusLabel_.setText("Enter a prompt", juce::dontSendNotification);
        return;
    }

    // Save before notes for diff
    diffBeforeNotes_ = pianoRoll_.getNotes();

    const auto barStart = pianoRoll_.getSelectedBarStart();
    const auto barEnd   = pianoRoll_.getSelectedBarEnd();

    statusLabel_.setText("Regenerating region...", juce::dontSendNotification);
    statusLabel_.setColour(juce::Label::textColourId, accentColor);
    regenButton_.setEnabled(false);
    generateButton_.setEnabled(false);

    CompositionRequest req;
    req.seed = static_cast<std::uint64_t>(seedSlider_.getValue());
    req.prompt = (prompt + " (regenerate bars " + std::to_string(barStart)
                  + "-" + std::to_string(barEnd) + ")").toStdString();

    auto result = proc_.engineBridge().generateVariations(req);

    if (result.success && !result.smf_midi.empty()) {
        auto newNotes = PianoRollComponent::parseSmfToNotes(result.smf_midi, 480);
        pianoRoll_.setNotes(newNotes);
        pianoRoll_.showDiff(diffBeforeNotes_, newNotes);
        pianoRoll_.clearSelection();

        diffMode_ = true;
        diffButton_.setButtonText("Hide Diff");
        diffButton_.setEnabled(true);
        statusLabel_.setText("Region regenerated", juce::dontSendNotification);
    } else {
        const auto& err = result.error;
        statusLabel_.setText(err.empty() ? "Regeneration failed" : err,
                             juce::dontSendNotification);
        diffBeforeNotes_.clear();
        diffMode_ = false;
        diffButton_.setButtonText("Show Diff");
        diffButton_.setEnabled(false);
    }

    statusLabel_.setColour(juce::Label::textColourId, dimColor);
    regenButton_.setEnabled(pianoRoll_.hasSelection()
                            && !promptBox_.getText().trim().isEmpty());
    generateButton_.setEnabled(true);
}

// ---------------------------------------------------------------------------
// Diff toggle
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::onDiffToggle() {
    diffMode_ = !diffMode_;
    if (diffMode_) {
        pianoRoll_.showDiff(diffBeforeNotes_, pianoRoll_.getNotes());
        diffButton_.setButtonText("Hide Diff");
    } else {
        pianoRoll_.clearDiff();
        diffButton_.setButtonText("Show Diff");
    }
}

// ---------------------------------------------------------------------------
// History helpers
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::addToHistory(const juce::String& prompt) {
    auto trimmed = prompt.trim();
    if (trimmed.isEmpty())
        return;

    promptHistory_.removeAllInstancesOf(trimmed);
    promptHistory_.insert(0, trimmed);

    while (promptHistory_.size() > 10)
        promptHistory_.removeLast();

    updateHistoryBox();
}

void AiMidiComposerEditor::updateHistoryBox() {
    historyBox_.clear(juce::dontSendNotification);
    for (auto& p : promptHistory_)
        historyBox_.addItem(p, historyBox_.getNumItems() + 1);
}

// ---------------------------------------------------------------------------
// Workflow list
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::updateWorkflowList() {
    workflowSelector_.clear(juce::dontSendNotification);
    workflowSelector_.addItem("New Composition", 1);
    workflowSelector_.addItem("Instrument Composer", 2);
    workflowSelector_.addItem("Audio Assisted", 3);
    workflowSelector_.addItem("Continue", 4);
    workflowSelector_.addItem("Smart Regeneration", 5);
    workflowSelector_.addItem("Generate Variations", 6);
    workflowSelector_.addItem("Replace Instrument", 7);
    workflowSelector_.addItem("Reharmonize", 8);
    workflowSelector_.addItem("Orchestrate", 9);
    workflowSelector_.addItem("Arrange", 10);
    workflowSelector_.setSelectedItemIndex(0, juce::dontSendNotification);
}

// ---------------------------------------------------------------------------
// State management
// ---------------------------------------------------------------------------
void AiMidiComposerEditor::loadState() {
    state_ = proc_.getState();
    if (!state_.isValid())
        state_ = juce::ValueTree("AiMidiComposerState");

    auto historyNode = state_.getChildWithName("history");
    if (historyNode.isValid()) {
        for (int i = 0; i < historyNode.getNumChildren(); ++i)
            promptHistory_.add(historyNode.getChild(i).getProperty("text").toString());
    }
    updateHistoryBox();

    auto lastPrompt = state_.getProperty("lastPrompt", {});
    if (lastPrompt.toString().isNotEmpty()) {
        promptBox_.setText(lastPrompt.toString(), juce::dontSendNotification);
        generateButton_.setEnabled(true);
    }

    auto lastWorkflow = static_cast<int>(state_.getProperty("lastWorkflow", 0));
    workflowSelector_.setSelectedItemIndex(
        juce::jlimit(0, workflowSelector_.getNumItems() - 1, lastWorkflow),
        juce::dontSendNotification);

    auto lastSeed = static_cast<double>(state_.getProperty("lastSeed", 42));
    seedSlider_.setValue(lastSeed, juce::dontSendNotification);
}

void AiMidiComposerEditor::saveState() {
    auto historyNode = state_.getOrCreateChildWithName("history", nullptr);
    historyNode.removeAllChildren(nullptr);
    for (auto& p : promptHistory_) {
        auto item = juce::ValueTree("item");
        item.setProperty("text", p, nullptr);
        historyNode.addChild(item, -1, nullptr);
    }

    state_.setProperty("lastPrompt", promptBox_.getText(), nullptr);
    state_.setProperty("lastWorkflow", workflowSelector_.getSelectedItemIndex(), nullptr);
    state_.setProperty("lastSeed", static_cast<int>(seedSlider_.getValue()), nullptr);

    proc_.setState(state_);
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
juce::AudioProcessorEditor* AiMidiComposerProcessor::createEditor() {
    return new AiMidiComposerEditor(*this);
}

} // namespace aimidi::plugin
