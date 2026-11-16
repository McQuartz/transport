#pragma once

#include <juce_graphics/juce_graphics.h>

namespace tp::defaultplayer
{
/** True on Windows and macOS. */
bool isSupported();

/** Whether Transport is already the default app for MP3 files. */
bool isDefault();

/** Windows: registers Transport as an audio player (per user) and opens the Default Apps settings
    page for it, where the user confirms. macOS: becomes the default viewer for common audio types.
    Returns a short message for the user. */
juce::String makeDefault();

/** Windows: if Transport registered its file types before, re-registers them so they point at this
    .exe and show its per-type icons (after an update or a move). Elsewhere a no-op.
    `iconColour` is the file-icon setting (see setIconColour). */
void refresh (const juce::String& iconColour);

/** True where the colour of the file-type icons can be changed (Windows). */
bool canColourIcons();

/** Windows: "auto" (light or dark, like Windows), a swatch id from docicon::swatches, or "#rrggbb".
    Makes the icons in that colour, registers the file types with them and tells Explorer.
    Returns a short message for the user. */
juce::String setIconColour (const juce::String& iconColour);

/** The page colour the setting stands for right now ("auto" follows the Windows theme). */
juce::Colour resolveIconColour (const juce::String& iconColour);
}
