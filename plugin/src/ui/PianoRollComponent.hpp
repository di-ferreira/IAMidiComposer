#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <string>

namespace aimidi::plugin {

// A single MIDI note for display.
struct PianoRollNote {
    int     tickOn;
    int     tickOff;
    int     note;
    int     velocity;
    int     channel;
    std::string trackName;
};

// A lock region marker.
struct LockRegion {
    int  barStart;
    int  barEnd;
    bool locked;
};

class PianoRollComponent : public juce::Component,
                           private juce::Timer {
public:
    PianoRollComponent();
    ~PianoRollComponent() override;

    void setNotes(const std::vector<PianoRollNote>& notes);
    void setLockRegions(const std::vector<LockRegion>& regions);
    void setTimeRange(int startTick, int endTick, int ppq, int bpm);
    void setVisibleRange(int lowNote, int highNote);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    
    // Drawing helpers
    void drawKeyboard(juce::Graphics& g, juce::Rectangle<int> area);
    void drawGrid(juce::Graphics& g, juce::Rectangle<int> area);
    void drawNotes(juce::Graphics& g, juce::Rectangle<int> area);
    void drawLockRegions(juce::Graphics& g, juce::Rectangle<int> area);
    void drawTooltip(juce::Graphics& g);
    void drawHeader(juce::Graphics& g, juce::Rectangle<int> headerArea, juce::Rectangle<int> gridArea);
    
    // Coordinate conversion
    int noteToY(int note, juce::Rectangle<int> area) const;
    int tickToX(int tick, juce::Rectangle<int> area) const;
    int yToNote(int y, juce::Rectangle<int> area) const;
    int xToTick(int x, juce::Rectangle<int> area) const;
    
    // Data
    std::vector<PianoRollNote> notes_;
    std::vector<LockRegion> lockRegions_;
    int startTick_ = 0;
    int endTick_ = 3840;  // 8 bars at 480 PPQ
    int ppq_ = 480;
    int bpm_ = 120;
    int lowNote_ = 36;   // C2
    int highNote_ = 84;  // C6
    
    // Interaction
    int hoverNote_ = -1;
    int hoverTick_ = -1;
    juce::Point<int> mousePos_;
    bool showTooltip_ = false;
    
    // Layout constants
    static constexpr int kKeyboardWidth = 50;
    static constexpr int kHeaderHeight = 20;
    static constexpr int kWhiteKeyHeight = 12;
    static constexpr int kBlackKeyWidth = 30;
    static constexpr float kBlackKeyHeightRatio = 0.6f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};

} // namespace aimidi::plugin
