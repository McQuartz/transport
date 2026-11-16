#pragma once

#include <juce_graphics/juce_graphics.h>

namespace tp::docicon
{
/** The file-type icon (a page with the type in big letters, the Transport logo small in the corner),
    drawn in any page colour. Same design as tools/make_doc_icons.py, which makes the default dark
    icons that ship inside the app; this one makes the coloured variants at runtime. */
juce::Image render (const juce::String& ext, bool video, juce::Colour page, int size);

/** A Windows .ico with the usual sizes (16 to 256, PNG-compressed). */
juce::MemoryBlock makeIco (const juce::String& ext, bool video, juce::Colour page);

/** The page colour of the icons that ship inside the app. */
inline const juce::Colour defaultPage { 0xff262626 };

/** Choices for the colour menu; the first one is the default. */
struct Swatch { const char* id; const char* name; juce::uint32 argb; };
inline constexpr Swatch swatches[] = {
    { "dark",     "Dark",     0xff262626 },
    { "graphite", "Graphite", 0xff4a4a4a },
    { "light",    "Light",    0xfff2f2f2 },
    { "orange",   "Orange",   0xffe8a33a },
    { "red",      "Red",      0xffd24b45 },
    { "green",    "Green",    0xff3f9f6a },
    { "blue",     "Blue",     0xff3d73d6 },
    { "purple",   "Purple",   0xff7c5cd1 },
};
} // namespace tp::docicon
