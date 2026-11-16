#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tp
{
/** What the system media overlay shows. */
struct NowPlaying
{
    juce::String title, artist;
    juce::Image cover;
    double length = 0.0, position = 0.0;
    bool playing = false, hasTrack = false;
};

/** The system's media controls: the keyboard's media keys, the Windows media overlay (volume flyout,
    lock screen) and macOS "Now Playing" in the menu bar / Control Centre.
    Windows: SystemMediaTransportControls. macOS: MPRemoteCommandCenter + MPNowPlayingInfoCenter.
    Linux: nothing yet (the media keys work while the window has the focus). */
class MediaControls : private juce::Timer
{
public:
    explicit MediaControls (juce::Component& topLevelWindow);
    ~MediaControls() override;

    std::function<void()> onPlayPause, onPlay, onPause, onNext, onPrev;
    std::function<NowPlaying()> state;   // polled twice a second

    struct Impl;

private:
    void timerCallback() override;
    std::unique_ptr<Impl> impl;
    juce::String lastKey;
    double lastPosition = -10.0;
};

namespace mediacontrols
{
/** The platform part; implemented in MediaControlsWindows.cpp / MediaControlsMac.mm / MediaControls.cpp. */
std::unique_ptr<MediaControls::Impl> create (juce::Component& window, MediaControls& owner);
void update (MediaControls::Impl&, const NowPlaying&, bool trackChanged);
void destroy (std::unique_ptr<MediaControls::Impl>&);
}
} // namespace tp
