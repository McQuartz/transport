#include "MediaControls.h"

namespace tp
{
MediaControls::MediaControls (juce::Component& window)
{
    impl = mediacontrols::create (window, *this);
    if (impl != nullptr) startTimer (500);
}

MediaControls::~MediaControls()
{
    stopTimer();
    mediacontrols::destroy (impl);
}

void MediaControls::timerCallback()
{
    if (impl == nullptr || ! state) return;
    const auto s = state();
    const auto key = s.title + "|" + s.artist + "|" + juce::String (s.length) + (s.playing ? "|p" : "|s");
    const bool trackChanged = key.upToLastOccurrenceOf ("|", false, false) != lastKey.upToLastOccurrenceOf ("|", false, false);
    // update when something changes, or when the position jumped (a seek), so the overlay's clock stays right
    const double expected = lastPosition + (s.playing ? 0.5 : 0.0);
    if (key != lastKey || std::abs (s.position - expected) > 1.5)
        mediacontrols::update (*impl, s, trackChanged);
    lastKey = key;
    lastPosition = s.position;
}

#if ! JUCE_WINDOWS && ! JUCE_MAC
struct MediaControls::Impl {};
void MediaControls::ImplDeleter::operator() (Impl* p) const { delete p; }
namespace mediacontrols
{
MediaControls::ImplPtr create (juce::Component&, MediaControls&) { return nullptr; }
void update (MediaControls::Impl&, const NowPlaying&, bool) {}
void destroy (MediaControls::ImplPtr& p) { p.reset(); }
}
#endif
} // namespace tp
