#include "ui/PianoRollComponent.hpp"
#include <juce_audio_basics/juce_audio_basics.h>
#include <algorithm>
#include <sstream>
#include <map>

namespace aimidi::plugin {

// ── Colour palette ──────────────────────────────────────────────────────────
namespace {
const auto kBgColor          = juce::Colour::fromRGB(24, 24, 28);
const auto kGridLine         = juce::Colour::fromRGBA(255, 255, 255, 12);
const auto kCNoteLine        = juce::Colour::fromRGBA(255, 255, 255, 35);
const auto kBeatLine         = juce::Colour::fromRGBA(255, 255, 255, 40);
const auto kBarLine          = juce::Colour::fromRGBA(255, 255, 255, 80);
const auto k16thDot          = juce::Colour::fromRGBA(255, 255, 255, 15);
const auto kHeaderBg         = juce::Colour::fromRGB(35, 35, 40);
const auto kHeaderText       = juce::Colour::fromRGBA(255, 255, 255, 100);
const auto kWhiteKey         = juce::Colour::fromRGB(240, 240, 235);
const auto kWhiteKeyEdge     = juce::Colour::fromRGB(200, 200, 195);
const auto kBlackKey         = juce::Colour::fromRGB(60, 60, 65);
const auto kBlackKeyEdge     = juce::Colour::fromRGB(80, 80, 85);
const auto kNoteLabel        = juce::Colour::fromRGB(80, 80, 80);
const auto kLockUnlocked     = juce::Colour::fromRGBA(0, 180, 0, 50);
const auto kLocked           = juce::Colour::fromRGBA(220, 40, 40, 60);
const auto kTooltipBg        = juce::Colour::fromRGBA(0, 0, 0, 210);
const auto kTooltipText      = juce::Colour::fromRGB(240, 240, 240);
const auto kHoverHighlight   = juce::Colour::fromRGBA(255, 255, 255, 20);
const auto kKeyboardBg       = juce::Colour::fromRGB(40, 40, 45);
const auto kSelectionFill    = juce::Colour::fromRGBA(0, 120, 255, 40);
const auto kSelectionBorder  = juce::Colour::fromRGBA(0, 120, 255, 180);
const auto kDiffBeforeFill   = juce::Colour::fromRGBA(180, 180, 180, 60);
const auto kDiffBeforeBorder = juce::Colour::fromRGBA(180, 180, 180, 100);

const juce::Colour kTrackColors[] = {
    juce::Colour::fromHSV(0.00f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.12f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.25f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.38f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.50f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.65f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.78f, 0.75f, 0.55f, 1.0f),
    juce::Colour::fromHSV(0.90f, 0.75f, 0.55f, 1.0f),
};
constexpr int kNumTrackColors = sizeof(kTrackColors) / sizeof(kTrackColors[0]);

bool isBlackKey(int note) {
    const int pc = note % 12;
    return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
}

bool isCNote(int note) {
    return (note % 12) == 0;
}

std::string noteName(int note) {
    static constexpr const char* names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    const int octave = (note / 12) - 1;
    std::ostringstream oss;
    oss << names[note % 12] << octave;
    return oss.str();
}

} // anonymous namespace

// ── Constructor / Destructor ────────────────────────────────────────────────
PianoRollComponent::PianoRollComponent() {
    startTimer(50);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

PianoRollComponent::~PianoRollComponent() = default;

// ── Public setters ──────────────────────────────────────────────────────────
void PianoRollComponent::setNotes(const std::vector<PianoRollNote>& notes) {
    notes_ = notes;
    repaint();
}

void PianoRollComponent::setLockRegions(const std::vector<LockRegion>& regions) {
    lockRegions_ = regions;
    repaint();
}

void PianoRollComponent::setTimeRange(int startTick, int endTick, int ppq, int bpm) {
    startTick_ = startTick;
    endTick_   = endTick;
    ppq_       = ppq;
    bpm_       = bpm;
    repaint();
}

void PianoRollComponent::setVisibleRange(int lowNote, int highNote) {
    lowNote_  = std::min(lowNote, highNote);
    highNote_ = std::max(lowNote, highNote);
    repaint();
}

// ── Diff visualization ──────────────────────────────────────────────────────
void PianoRollComponent::showDiff(const std::vector<PianoRollNote>& before,
                                   const std::vector<PianoRollNote>& after) {
    beforeNotes_ = before;
    afterNotes_  = after;
    diffMode_    = true;
    repaint();
}

void PianoRollComponent::clearDiff() {
    beforeNotes_.clear();
    afterNotes_.clear();
    diffMode_ = false;
    repaint();
}

// ── Region selection ────────────────────────────────────────────────────────
void PianoRollComponent::clearSelection() {
    selectedBarStart_ = -1;
    selectedBarEnd_   = -1;
    isDragging_       = false;
    isDragExport_     = false;
    repaint();
}

bool PianoRollComponent::hasSelection() const {
    return selectedBarStart_ >= 0 && selectedBarEnd_ > selectedBarStart_;
}

int PianoRollComponent::getSelectedBarStart() const {
    return selectedBarStart_;
}

int PianoRollComponent::getSelectedBarEnd() const {
    return selectedBarEnd_;
}

// ── SMF parsing ─────────────────────────────────────────────────────────────
std::vector<PianoRollNote> PianoRollComponent::parseSmfToNotes(
    const std::vector<std::uint8_t>& smfData, int ppq)
{
    std::vector<PianoRollNote> notes;

    juce::MemoryInputStream stream(smfData.data(), smfData.size(), false);
    juce::MidiFile midiFile;
    if (!midiFile.readFrom(stream))
        return notes;

    for (int t = 0; t < midiFile.getNumTracks(); ++t) {
        auto* track = midiFile.getTrack(t);
        if (!track) continue;

        std::string trackName;
        for (int i = 0; i < track->getNumEvents(); ++i) {
            auto& msg = track->getEventPointer(i)->message;
            if (msg.isTrackNameEvent())
                trackName = msg.getTextFromTextMetaEvent().toStdString();
        }

        for (int i = 0; i < track->getNumEvents(); ++i) {
            auto* ev = track->getEventPointer(i);
            if (!ev->message.isNoteOn() || ev->message.getVelocity() == 0)
                continue;

            int note     = ev->message.getNoteNumber();
            int velocity = ev->message.getVelocity();
            int channel  = ev->message.getChannel();
            int tickOn   = static_cast<int>(ev->message.getTimeStamp());
            int tickOff  = tickOn + ppq;

            for (int j = i + 1; j < track->getNumEvents(); ++j) {
                auto* off = track->getEventPointer(j);
                if (off->message.isNoteOff() && off->message.getNoteNumber() == note) {
                    tickOff = static_cast<int>(off->message.getTimeStamp());
                    break;
                }
            }

            notes.push_back({tickOn, tickOff, note, velocity, channel, trackName});
        }
    }

    return notes;
}

// ── JUCE overrides ──────────────────────────────────────────────────────────
void PianoRollComponent::paint(juce::Graphics& g) {
    g.fillAll(kBgColor);

    auto bounds      = getLocalBounds();
    auto headerArea  = bounds.removeFromTop(kHeaderHeight);
    auto keyArea     = bounds.removeFromLeft(kKeyboardWidth);
    auto gridArea    = bounds;

    drawHeader(g, headerArea, gridArea);
    drawKeyboard(g, keyArea);
    drawGrid(g, gridArea);
    drawLockRegions(g, gridArea);
    drawSelection(g, gridArea);
    if (diffMode_) {
        drawDiffNotes(g, gridArea);
    } else {
        drawNotes(g, gridArea);
    }
    drawTooltip(g);
}

void PianoRollComponent::resized() {}

// ── Mouse ───────────────────────────────────────────────────────────────────
void PianoRollComponent::mouseMove(const juce::MouseEvent& event) {
    mousePos_ = event.getPosition();
    showTooltip_ = true;

    auto bounds     = getLocalBounds();
    auto headerArea = bounds.removeFromTop(kHeaderHeight);
    auto keyArea    = bounds.removeFromLeft(kKeyboardWidth);
    auto gridArea   = bounds;

    if (gridArea.contains(mousePos_)) {
        hoverNote_ = yToNote(mousePos_.y, gridArea);
        hoverTick_ = xToTick(mousePos_.x, gridArea);
    } else if (keyArea.contains(mousePos_)) {
        hoverNote_ = yToNote(mousePos_.y, keyArea);
        hoverTick_ = -1;
    } else {
        hoverNote_  = -1;
        hoverTick_  = -1;
        showTooltip_ = false;
    }

    repaint();
}

void PianoRollComponent::mouseExit(const juce::MouseEvent&) {
    showTooltip_ = false;
    hoverNote_   = -1;
    hoverTick_   = -1;
    repaint();
}

void PianoRollComponent::mouseDown(const juce::MouseEvent& event) {
    auto bounds     = getLocalBounds();
    auto headerArea = bounds.removeFromTop(kHeaderHeight);
    auto keyArea    = bounds.removeFromLeft(kKeyboardWidth);
    auto gridArea   = bounds;

    const auto pos = event.getPosition();

    if (headerArea.contains(pos) || gridArea.contains(pos)) {
        // Start region selection
        isDragging_   = true;
        isDragExport_ = false;
        dragStartTick_ = xToTick(pos.x, gridArea);

        const int ticksPerBar = ppq_ * 4;
        const int bar = std::clamp(dragStartTick_ / ticksPerBar,
                                   0, endTick_ / ticksPerBar);
        selectedBarStart_ = bar;
        selectedBarEnd_   = bar;
        repaint();
    } else if (keyArea.contains(pos)) {
        // Prepare drag export
        isDragExport_ = true;
        isDragging_   = false;
        dragStartPos_ = pos;
    }
}

void PianoRollComponent::mouseDrag(const juce::MouseEvent& event) {
    auto bounds     = getLocalBounds();
    auto headerArea = bounds.removeFromTop(kHeaderHeight);
    auto keyArea    = bounds.removeFromLeft(kKeyboardWidth);
    auto gridArea   = bounds;

    const auto pos = event.getPosition();

    if (isDragExport_) {
        const auto diff = pos - dragStartPos_;
        if (diff.getDistanceFromOrigin() > 8) {
            startSMFExport();
            isDragExport_ = false;
            isDragging_   = false;
        }
        return;
    }

    if (isDragging_) {
        const int currentTick = xToTick(pos.x, gridArea);
        const int ticksPerBar = ppq_ * 4;
        const int startBar = std::clamp(dragStartTick_ / ticksPerBar,
                                        0, endTick_ / ticksPerBar);
        const int endBar   = std::clamp(currentTick / ticksPerBar,
                                        0, endTick_ / ticksPerBar);

        selectedBarStart_ = std::min(startBar, endBar);
        selectedBarEnd_   = std::max(startBar, endBar);
        repaint();
    }
}

void PianoRollComponent::mouseUp(const juce::MouseEvent&) {
    isDragging_   = false;
    isDragExport_ = false;
    repaint();

    if (onSelectionChanged)
        onSelectionChanged();
}

void PianoRollComponent::timerCallback() {
    if (isMouseOver(true))
        repaint();
}

// ── Coordinate conversion ───────────────────────────────────────────────────
int PianoRollComponent::noteToY(int note, juce::Rectangle<int> area) const {
    const int idx = note - lowNote_;
    return area.getBottom() - (idx + 1) * kWhiteKeyHeight;
}

int PianoRollComponent::tickToX(int tick, juce::Rectangle<int> area) const {
    const double range   = static_cast<double>(endTick_ - startTick_);
    if (range <= 0.0)
        return area.getX();
    const double fraction = static_cast<double>(tick - startTick_) / range;
    return area.getX() + static_cast<int>(fraction * area.getWidth());
}

int PianoRollComponent::yToNote(int y, juce::Rectangle<int> area) const {
    const int idx = (area.getBottom() - y) / kWhiteKeyHeight;
    return std::clamp(lowNote_ + idx, lowNote_, highNote_);
}

int PianoRollComponent::xToTick(int x, juce::Rectangle<int> area) const {
    const double range = static_cast<double>(endTick_ - startTick_);
    if (range <= 0.0)
        return startTick_;
    const double fraction = static_cast<double>(x - area.getX()) / area.getWidth();
    return static_cast<int>(startTick_ + fraction * range);
}

// ── Drawing helpers ─────────────────────────────────────────────────────────
void PianoRollComponent::drawKeyboard(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(kKeyboardBg);
    g.fillRect(area);

    for (int note = lowNote_; note <= highNote_; ++note) {
        const auto keyArea = juce::Rectangle<int>{
            area.getX(), noteToY(note, area),
            area.getWidth(), kWhiteKeyHeight
        };

        if (isBlackKey(note)) {
            const auto bk = juce::Rectangle<int>{
                area.getX(), keyArea.getY(),
                kBlackKeyWidth,
                static_cast<int>(kWhiteKeyHeight * kBlackKeyHeightRatio)
            };
            g.setColour(kBlackKey);
            g.fillRect(bk);
            g.setColour(kBlackKeyEdge);
            g.drawRect(bk, 1);

            if (note == hoverNote_) {
                g.setColour(kHoverHighlight);
                g.fillRect(keyArea);
            }
        } else {
            g.setColour(kWhiteKey);
            g.fillRect(keyArea);
            g.setColour(kWhiteKeyEdge);
            g.drawRect(keyArea, 1);

            if (note == hoverNote_) {
                g.setColour(kHoverHighlight);
                g.fillRect(keyArea);
            }

            if (isCNote(note)) {
                g.setColour(kNoteLabel);
                g.setFont(9.0f);
                g.drawText(noteName(note),
                           keyArea.withTrimmedLeft(3),
                           juce::Justification::centredLeft);
            }
        }
    }

    g.setColour(juce::Colour::fromRGB(60, 60, 65));
    g.drawVerticalLine(area.getRight() - 1, area.getY(), area.getBottom());
}

void PianoRollComponent::drawGrid(juce::Graphics& g, juce::Rectangle<int> area) {
    for (int note = lowNote_; note <= highNote_; ++note) {
        const int y = noteToY(note, area);
        const bool isC = isCNote(note);
        g.setColour(isC ? kCNoteLine : kGridLine);
        g.drawHorizontalLine(y + kWhiteKeyHeight - 1, area.getX(), area.getRight());
    }

    const int ticksPerBeat  = ppq_;
    const int ticksPerBar   = ppq_ * 4;
    const int ticksPer16th  = ppq_ / 4;

    for (int tick = startTick_; tick <= endTick_; tick += ticksPer16th) {
        const int x = tickToX(tick, area);
        const bool isBar   = (tick % ticksPerBar == 0);
        const bool isBeat  = (tick % ticksPerBeat == 0);

        if (isBar) {
            g.setColour(kBarLine);
            g.drawVerticalLine(x, area.getY(), area.getBottom());
        } else if (isBeat) {
            g.setColour(kBeatLine);
            g.drawVerticalLine(x, area.getY(), area.getBottom());
        } else {
            g.setColour(k16thDot);
            g.drawVerticalLine(x, area.getY(), area.getBottom());
        }
    }
}

void PianoRollComponent::drawNotes(juce::Graphics& g, juce::Rectangle<int> area) {
    for (const auto& note : notes_) {
        if (note.note < lowNote_ || note.note > highNote_)
            continue;
        if (note.tickOff <= startTick_ || note.tickOn >= endTick_)
            continue;

        int x  = tickToX(note.tickOn, area);
        int x2 = tickToX(note.tickOff, area);
        int w  = std::max(2, x2 - x);
        int y  = noteToY(note.note, area);

        const auto& base = kTrackColors[note.channel % kNumTrackColors];
        const float alpha = 0.30f + (std::clamp(note.velocity, 0, 127) / 127.0f) * 0.70f;

        g.setColour(base.withAlpha(alpha));
        g.fillRect(x, y, w, kWhiteKeyHeight);

        g.setColour(base.withAlpha(1.0f));
        g.drawRect(x, y, w, kWhiteKeyHeight, 1);
    }
}

void PianoRollComponent::drawSelection(juce::Graphics& g, juce::Rectangle<int> area) {
    if (selectedBarStart_ < 0 || selectedBarEnd_ < 0)
        return;

    const int ticksPerBar = ppq_ * 4;
    const int x1 = tickToX(selectedBarStart_ * ticksPerBar, area);
    const int x2 = tickToX((selectedBarEnd_ + 1) * ticksPerBar, area);

    if (x2 <= x1)
        return;

    g.setColour(kSelectionFill);
    g.fillRect(x1, area.getY(), x2 - x1, area.getHeight());

    g.setColour(kSelectionBorder);
    g.drawRect(x1, area.getY(), x2 - x1, area.getHeight(), 1);
}

void PianoRollComponent::drawDiffNotes(juce::Graphics& g, juce::Rectangle<int> area) {
    // Draw before notes in gray (semi-transparent)
    for (const auto& note : beforeNotes_) {
        if (note.note < lowNote_ || note.note > highNote_)
            continue;
        if (note.tickOff <= startTick_ || note.tickOn >= endTick_)
            continue;

        int x  = tickToX(note.tickOn, area);
        int x2 = tickToX(note.tickOff, area);
        int w  = std::max(2, x2 - x);
        int y  = noteToY(note.note, area);

        g.setColour(kDiffBeforeFill);
        g.fillRect(x, y, w, kWhiteKeyHeight);
        g.setColour(kDiffBeforeBorder);
        g.drawRect(x, y, w, kWhiteKeyHeight, 1);
    }

    // Draw after notes in color
    for (const auto& note : afterNotes_) {
        if (note.note < lowNote_ || note.note > highNote_)
            continue;
        if (note.tickOff <= startTick_ || note.tickOn >= endTick_)
            continue;

        int x  = tickToX(note.tickOn, area);
        int x2 = tickToX(note.tickOff, area);
        int w  = std::max(2, x2 - x);
        int y  = noteToY(note.note, area);

        const auto& base = kTrackColors[note.channel % kNumTrackColors];
        const float alpha = 0.30f + (std::clamp(note.velocity, 0, 127) / 127.0f) * 0.70f;

        g.setColour(base.withAlpha(alpha));
        g.fillRect(x, y, w, kWhiteKeyHeight);

        g.setColour(base.withAlpha(1.0f));
        g.drawRect(x, y, w, kWhiteKeyHeight, 1);
    }
}

void PianoRollComponent::drawLockRegions(juce::Graphics& g, juce::Rectangle<int> area) {
    if (lockRegions_.empty())
        return;

    const int ticksPerBar = ppq_ * 4;

    for (const auto& region : lockRegions_) {
        const int x1 = tickToX(region.barStart * ticksPerBar, area);
        const int x2 = tickToX(region.barEnd   * ticksPerBar, area);
        if (x2 <= x1)
            continue;

        g.setColour(region.locked ? kLocked : kLockUnlocked);
        g.fillRect(x1, area.getY(), x2 - x1, area.getHeight());

        g.setColour(juce::Colour::fromRGBA(255, 255, 255, 120));
        g.setFont(10.0f);
        g.drawText(region.locked ? "LOCKED" : "UNLOCKED",
                   x1, area.getY(), x2 - x1, kHeaderHeight,
                   juce::Justification::centred);
    }
}

void PianoRollComponent::drawHeader(juce::Graphics& g,
                                     juce::Rectangle<int> headerArea,
                                     juce::Rectangle<int> gridArea) {
    g.setColour(kHeaderBg);
    g.fillRect(headerArea);

    const int ticksPerBar  = ppq_ * 4;
    const int ticksPerBeat = ppq_;

    g.setFont(10.0f);
    g.setColour(kHeaderText);
    for (int tick = startTick_; tick <= endTick_; tick += ticksPerBar) {
        const int x = tickToX(tick, gridArea);
        const int barNum = tick / ticksPerBar + 1;
        g.drawText(std::to_string(barNum),
                   x + 3, headerArea.getY(),
                   30, headerArea.getHeight(),
                   juce::Justification::centredLeft);
    }

    g.setFont(8.0f);
    for (int tick = startTick_; tick <= endTick_; tick += ticksPerBeat) {
        if (tick % ticksPerBar == 0)
            continue;
        const int x = tickToX(tick, gridArea);
        const int beatInBar = (tick % ticksPerBar) / ticksPerBeat;
        g.drawText(std::to_string(beatInBar + 1),
                   x + 1, headerArea.getY() + 10,
                   15, headerArea.getHeight() - 10,
                   juce::Justification::centredLeft);
    }
}

void PianoRollComponent::drawTooltip(juce::Graphics& g) {
    if (!showTooltip_)
        return;

    if (hoverNote_ < 0) {
        std::ostringstream oss;
        oss << "t=" << hoverTick_;
        const auto text = oss.str();

        g.setFont(12.0f);
        const int tw = g.getCurrentFont().getStringWidth(text) + 16;
        const int th = 22;
        auto rect = juce::Rectangle<int>(mousePos_.x + 15, mousePos_.y - 10, tw, th);
        rect = rect.constrainedWithin(getLocalBounds());

        g.setColour(kTooltipBg);
        g.fillRoundedRectangle(rect.toFloat(), 4.0f);
        g.setColour(kTooltipText);
        g.drawText(text, rect, juce::Justification::centred);
        return;
    }

    const auto it = std::find_if(notes_.begin(), notes_.end(),
        [this](const PianoRollNote& n) {
            return n.note == hoverNote_
                && n.tickOn <= hoverTick_
                && n.tickOff > hoverTick_;
        });

    std::string text;
    if (it != notes_.end()) {
        std::ostringstream oss;
        oss << noteName(it->note)
            << "  t=" << it->tickOn << "-" << it->tickOff
            << "  v=" << it->velocity
            << "  ch=" << (it->channel + 1);
        if (!it->trackName.empty())
            oss << "  " << it->trackName;
        text = oss.str();
    } else {
        text = noteName(hoverNote_);
    }

    g.setFont(12.0f);
    const int tw = g.getCurrentFont().getStringWidth(text) + 16;
    const int th = 22;
    auto rect = juce::Rectangle<int>(mousePos_.x + 15, mousePos_.y - 10, tw, th);
    rect = rect.constrainedWithin(getLocalBounds());

    g.setColour(kTooltipBg);
    g.fillRoundedRectangle(rect.toFloat(), 4.0f);
    g.setColour(kTooltipText);
    g.drawText(text, rect, juce::Justification::centred);
}

// ── SMF Export (drag-and-drop) ──────────────────────────────────────────────
void PianoRollComponent::startSMFExport() {
    if (notes_.empty())
        return;

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ppq_);

    // Conductor track
    juce::MidiMessageSequence conductorTrack;
    const int tempo = 60000000 / bpm_;
    conductorTrack.addEvent(juce::MidiMessage::tempoMetaEvent(tempo), 0);
    conductorTrack.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0);
    midiFile.addTrack(conductorTrack);

    // Group notes by channel
    std::map<int, juce::MidiMessageSequence> trackSequences;
    for (const auto& note : notes_) {
        auto& seq = trackSequences[note.channel];
        seq.addEvent(juce::MidiMessage::noteOn(note.channel, note.note,
            (juce::uint8)note.velocity), note.tickOn);
        seq.addEvent(juce::MidiMessage::noteOff(note.channel, note.note,
            (juce::uint8)0), note.tickOff);
    }

    for (auto& [ch, seq] : trackSequences) {
        seq.sort();
        midiFile.addTrack(seq);
    }

    // Write to temp file and start external file drag
    auto tempFile = juce::File::createTempFile(".mid");
    if (auto out = tempFile.createOutputStream()) {
        if (midiFile.writeTo(*out)) {
            out->flush();
            out.reset();
            juce::StringArray files;
            files.add(tempFile.getFullPathName());
            juce::DragAndDropContainer::performExternalDragDropOfFiles(files, false);
        }
    }
}

} // namespace aimidi::plugin
