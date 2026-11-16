#pragma once

#include "Theme.h"
#include <cstring>

namespace tp
{
enum class QuickAction
{
    monitor, playlist, addVisualizer, dim,
    open, mono, swap, soloLeft, soloRight, mid, side, bypass,
    abLoop, alwaysOnTop, compact, snapshot, customize, video, fullscreen,
    count
};

struct QuickActionInfo
{
    QuickAction action;
    const char* id;
    const char* name;
    const char* description;
    bool toggle;
};

inline const std::vector<QuickActionInfo>& quickActions()
{
    static const std::vector<QuickActionInfo> list {
        { QuickAction::monitor,       "monitor",  "Monitoring",        "Mono, swap, solo, mid/side in one menu", true },
        { QuickAction::playlist,      "playlist", "Playlist",          "Jump to any track in this folder",       false },
        { QuickAction::addVisualizer, "addvis",   "Add plugin",        "Open the plugin browser",                false },
        { QuickAction::dim,           "dim",      "Dim",               "Drop the level by 20 dB",                true },
        { QuickAction::open,          "open",     "Open file",         "Open a track",                           false },
        { QuickAction::mono,          "mono",     "Mono check",        "Sum to mono, reveals phase problems",    true },
        { QuickAction::swap,          "swap",     "Swap L/R",          "Swap left and right",                    true },
        { QuickAction::soloLeft,      "soloL",    "Solo left",         "Hear the left channel only",             true },
        { QuickAction::soloRight,     "soloR",    "Solo right",        "Hear the right channel only",            true },
        { QuickAction::mid,           "mid",      "Mid solo",          "Only what is in the centre",             true },
        { QuickAction::side,          "side",     "Side solo",         "Only the stereo information",            true },
        { QuickAction::bypass,        "bypass",   "Bypass",            "Everything off, untouched signal",       true },
        { QuickAction::abLoop,        "ab",       "A-B loop",          "Click: set A, set B, clear",             true },
        { QuickAction::alwaysOnTop,   "ontop",    "Always on top",     "Keep the player above other windows",    true },
        { QuickAction::compact,       "compact",  "Compact",           "Hide the plugin row",                    true },
        { QuickAction::snapshot,      "snapshot", "Snapshot",          "Save the window as a PNG",               false },
        { QuickAction::customize,     "customize","Customize layout",  "Add, arrange and size plugins and areas", true },
        { QuickAction::video,         "video",    "Video / plugins",   "Show the video or the plugins below the waveform", true },
        { QuickAction::fullscreen,    "fullscreen","Full screen video","Play the video on the whole screen",     false },
    };
    return list;
}

inline const QuickActionInfo& infoFor (QuickAction a)
{
    for (auto& i : quickActions())
        if (i.action == a) return i;
    return quickActions().front();
}

inline QuickAction actionFromId (const juce::String& id, QuickAction fallback)
{
    for (auto& i : quickActions())
        if (id == i.id) return i.action;
    return fallback;
}

/** Which icon set to draw: 0 = the original set, 1 = new set with faders, 2 = new set with headphones
    (shipped). Kept so --quick-icons can show the designs side by side. */
inline int& quickIconVariant() { static int v = 2; return v; }   // 2 = the current set (headphones)

/** Draws the button glyph. Icons for the prototype's four buttons, compact monograms for the rest. */
inline void drawQuickActionIcon (juce::Graphics& g, QuickAction a, juce::Rectangle<float> r, juce::Colour c)
{
    const auto centre = r.getCentre();
    g.setColour (c);
    juce::Path p;

    const bool proposal = quickIconVariant() > 0;
    auto monogram = [&] (const char* text, float px = 12.0f)
    {
        // proposal: every letter label the same size and weight, so they read as one family
        if (proposal) px = std::strlen (text) == 1 ? 17.0f : 13.0f;
        g.setFont (theme::mono (px));
        g.drawText (utf8 (text), r, juce::Justification::centred, false);
    };

    switch (a)
    {
        case QuickAction::monitor:
        {
            // 0 = the original (knobs filled with the tile colour), 1 = cleaned-up faders (solid caps, track
            // broken around them, like the Dim knob), 2 = headphones
            if (quickIconVariant() == 0)
            {
                const float knobs[] = { 18.0f, 22.0f, 8.0f };
                for (int i = 0; i < 3; ++i)
                {
                    const float x = centre.x - 10.0f + (float) i * 10.0f;
                    g.drawLine (x, centre.y - 14, x, centre.y + 14, 1.8f);
                    auto knob = juce::Rectangle<float> (8, 5).withCentre ({ x, centre.y - 14 + knobs[i] + 2.5f - 6.0f });
                    g.setColour (theme::button);
                    g.fillRect (knob);
                    g.setColour (c);
                    g.drawRect (knob, 1.6f);
                }
            }
            else if (quickIconVariant() == 1)
            {
                const float caps[] = { 4.0f, -6.0f, 7.0f };
                for (int i = 0; i < 3; ++i)
                {
                    const float x = centre.x - 11.0f + (float) i * 11.0f, y = centre.y + caps[i];
                    juce::Path track;
                    track.startNewSubPath (x, centre.y - 14); track.lineTo (x, y - 6.5f);
                    track.startNewSubPath (x, y + 6.5f);      track.lineTo (x, centre.y + 14);
                    g.strokePath (track, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                    g.fillRoundedRectangle (juce::Rectangle<float> (9.0f, 8.0f).withCentre ({ x, y }), 2.5f);
                }
            }
            else
            {
                juce::Path band;
                band.addCentredArc (centre.x, centre.y + 2, 13, 13, 0, -1.75f, 1.75f, true);
                g.strokePath (band, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.fillRoundedRectangle (centre.x - 15.5f, centre.y + 1, 7.5f, 13, 3.0f);
                g.fillRoundedRectangle (centre.x + 8.0f,  centre.y + 1, 7.5f, 13, 3.0f);
            }
            break;
        }
        case QuickAction::playlist:
            for (int i = -1; i <= 1; ++i)
                g.drawLine (centre.x - 15, centre.y + (float) i * 10.0f, centre.x + 15, centre.y + (float) i * 10.0f, 2.0f);
            break;
        case QuickAction::addVisualizer:
        {
            const float h[] = { 10, 14, 18, 15, 22 };
            const int n = proposal ? 3 : 5;
            for (int i = 0; i < n; ++i)
                g.fillRect (juce::Rectangle<float> (centre.x - (proposal ? 18.0f : 16.0f) + (float) i * 6.5f, centre.y + 11.0f - h[i], 5.0f, h[i]));
            if (proposal)   // bars + a plus: "add a meter"
            {
                g.drawLine (centre.x + 11.0f, centre.y - 5.0f, centre.x + 11.0f, centre.y + 9.0f, 2.2f);
                g.drawLine (centre.x + 4.0f, centre.y + 2.0f, centre.x + 18.0f, centre.y + 2.0f, 2.2f);
            }
            break;
        }
        case QuickAction::dim:   // the knob
        {
            g.drawEllipse (juce::Rectangle<float> (32, 32).withCentre (centre), 1.6f);
            g.fillEllipse (juce::Rectangle<float> (22, 22).withCentre (centre));
            g.setColour (theme::button);
            g.fillEllipse (juce::Rectangle<float> (4.4f, 4.4f).withCentre (centre.translated (-4.5f, -4.0f)));
            break;
        }
        case QuickAction::open:
            p.startNewSubPath (centre.x - 15, centre.y - 9);
            p.lineTo (centre.x - 5, centre.y - 9); p.lineTo (centre.x - 2, centre.y - 5); p.lineTo (centre.x + 15, centre.y - 5);
            p.lineTo (centre.x + 15, centre.y + 11); p.lineTo (centre.x - 15, centre.y + 11); p.closeSubPath();
            g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::mitered));
            break;
        case QuickAction::mono:
            if (proposal) monogram ("MONO");   // a bare circle reads as "record"
            else g.drawEllipse (juce::Rectangle<float> (22, 22).withCentre (centre), 2.0f);
            break;
        case QuickAction::swap:
            p.startNewSubPath (centre.x - 13, centre.y - 5); p.lineTo (centre.x + 13, centre.y - 5);
            p.startNewSubPath (centre.x + 7, centre.y - 11); p.lineTo (centre.x + 13, centre.y - 5); p.lineTo (centre.x + 7, centre.y + 1);
            p.startNewSubPath (centre.x + 13, centre.y + 7); p.lineTo (centre.x - 13, centre.y + 7);
            p.startNewSubPath (centre.x - 7, centre.y + 1); p.lineTo (centre.x - 13, centre.y + 7); p.lineTo (centre.x - 7, centre.y + 13);
            g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        case QuickAction::soloLeft:  monogram ("L", 18); break;
        case QuickAction::soloRight: monogram ("R", 18); break;
        case QuickAction::mid:       monogram ("MID"); break;
        case QuickAction::side:      monogram ("SIDE"); break;
        case QuickAction::bypass:    monogram ("BYP"); break;
        case QuickAction::abLoop:    monogram ("A\xe2\x80\x93" "B", 13); break;
        case QuickAction::alwaysOnTop:
            if (proposal)   // a pin, the usual sign for "keep on top"
            {
                p.startNewSubPath (centre.x - 6, centre.y - 13); p.lineTo (centre.x + 6, centre.y - 13);
                p.startNewSubPath (centre.x - 4, centre.y - 13); p.lineTo (centre.x - 4, centre.y - 3);
                p.lineTo (centre.x - 9, centre.y + 3); p.lineTo (centre.x + 9, centre.y + 3); p.lineTo (centre.x + 4, centre.y - 3);
                p.lineTo (centre.x + 4, centre.y - 13);
                p.startNewSubPath (centre.x, centre.y + 3); p.lineTo (centre.x, centre.y + 14);
                g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                break;
            }
            p.addRectangle (centre.x - 13, centre.y - 13, 26, 4);
            p.startNewSubPath (centre.x, centre.y - 7); p.lineTo (centre.x, centre.y + 13);
            p.startNewSubPath (centre.x - 7, centre.y); p.lineTo (centre.x, centre.y - 7); p.lineTo (centre.x + 7, centre.y);
            g.strokePath (p, juce::PathStrokeType (1.8f));
            break;
        case QuickAction::compact:
            p.addRoundedRectangle (centre.x - 15, centre.y - 11, 30, 22, 2.0f);
            p.startNewSubPath (centre.x - 15, centre.y - 2); p.lineTo (centre.x + 15, centre.y - 2);
            g.strokePath (p, juce::PathStrokeType (1.8f));
            break;
        case QuickAction::snapshot:
            p.addRoundedRectangle (centre.x - 15, centre.y - 8, 30, 20, 3.0f);
            p.addEllipse (juce::Rectangle<float> (12, 12).withCentre (centre.translated (0, 2)));
            p.addRectangle (centre.x - 6, centre.y - 12, 12, 4);
            g.strokePath (p, juce::PathStrokeType (1.8f));
            break;
        case QuickAction::customize:
            for (int i = 0; i < 4; ++i)
                g.drawRoundedRectangle (juce::Rectangle<float> (11, 11).withCentre (centre.translated ((i % 2) ? 7.0f : -7.0f, (i / 2) ? 7.0f : -7.0f)), 2.0f, 1.8f);
            break;
        case QuickAction::video:
            p.addRoundedRectangle (centre.x - 14, centre.y - 10, 28, 20, 3.0f);
            g.strokePath (p, juce::PathStrokeType (1.8f));
            p.clear();
            p.addTriangle (centre.x - 3.5f, centre.y - 5.0f, centre.x + 5.5f, centre.y, centre.x - 3.5f, centre.y + 5.0f);
            g.fillPath (p);
            break;
        case QuickAction::fullscreen:
            for (int i = 0; i < 4; ++i)
            {
                const float sx = (i % 2) ? 1.0f : -1.0f, sy = (i / 2) ? 1.0f : -1.0f;
                const auto corner = centre.translated (sx * 12.0f, sy * 9.0f);
                p.startNewSubPath (corner.translated (-sx * 7.0f, 0));
                p.lineTo (corner);
                p.lineTo (corner.translated (0, -sy * 6.0f));
            }
            g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            break;
        case QuickAction::count:
            break;
    }
}
} // namespace tp
