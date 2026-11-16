// Windows media overlay and media keys through SystemMediaTransportControls (C++/WinRT from the Windows SDK).
#include "MediaControls.h"

#if JUCE_WINDOWS && __has_include(<winrt/Windows.Media.h>) && __has_include(<systemmediatransportcontrolsinterop.h>)
 #define TP_HAS_SMTC 1
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #undef GetCurrentTime
 #include <winrt/base.h>
 #include <winrt/Windows.Foundation.h>
 #include <winrt/Windows.Media.h>
 #include <winrt/Windows.Storage.h>
 #include <winrt/Windows.Storage.Streams.h>
 #include <systemmediatransportcontrolsinterop.h>
 #include <thread>
#endif

#if JUCE_WINDOWS
namespace tp
{
#if TP_HAS_SMTC
using namespace winrt::Windows::Media;

struct MediaControls::Impl
{
    SystemMediaTransportControls smtc { nullptr };
    winrt::event_token token {};
    juce::File coverFile;
    int coverCounter = 0;
};

namespace mediacontrols
{
std::unique_ptr<MediaControls::Impl> create (juce::Component& window, MediaControls& owner)
{
    try
    {
        auto hwnd = (HWND) window.getWindowHandle();
        if (hwnd == nullptr) return nullptr;
        auto interop = winrt::get_activation_factory<SystemMediaTransportControls, ISystemMediaTransportControlsInterop>();
        auto impl = std::make_unique<MediaControls::Impl>();
        winrt::check_hresult (interop->GetForWindow (hwnd, winrt::guid_of<SystemMediaTransportControls>(), winrt::put_abi (impl->smtc)));
        auto& s = impl->smtc;
        s.IsEnabled (true);
        s.IsPlayEnabled (true);
        s.IsPauseEnabled (true);
        s.IsNextEnabled (true);
        s.IsPreviousEnabled (true);
        s.PlaybackStatus (MediaPlaybackStatus::Stopped);

        juce::Component::SafePointer<juce::Component> safeWindow (&window);
        auto* o = &owner;
        impl->token = s.ButtonPressed ([safeWindow, o] (auto&&, SystemMediaTransportControlsButtonPressedEventArgs const& args)
        {
            const auto button = args.Button();   // arrives on a system thread
            juce::MessageManager::callAsync ([safeWindow, o, button]
            {
                if (safeWindow == nullptr) return;
                switch (button)
                {
                    case SystemMediaTransportControlsButton::Play:     if (o->onPlay) o->onPlay(); break;
                    case SystemMediaTransportControlsButton::Pause:    if (o->onPause) o->onPause(); break;
                    case SystemMediaTransportControlsButton::Next:     if (o->onNext) o->onNext(); break;
                    case SystemMediaTransportControlsButton::Previous: if (o->onPrev) o->onPrev(); break;
                    default: break;
                }
            });
        });
        impl->coverFile = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("Transport-cover");
        return impl;
    }
    catch (...)
    {
        return nullptr;
    }
}

void update (MediaControls::Impl& impl, const NowPlaying& np, bool trackChanged)
{
    try
    {
        auto& s = impl.smtc;
        s.PlaybackStatus (! np.hasTrack ? MediaPlaybackStatus::Stopped
                                        : (np.playing ? MediaPlaybackStatus::Playing : MediaPlaybackStatus::Paused));

        // the overlay's progress bar
        SystemMediaTransportControlsTimelineProperties timeline;
        using namespace std::chrono;
        timeline.StartTime (winrt::Windows::Foundation::TimeSpan (0));
        timeline.EndTime (duration_cast<winrt::Windows::Foundation::TimeSpan> (duration<double> (np.length)));
        timeline.Position (duration_cast<winrt::Windows::Foundation::TimeSpan> (duration<double> (np.position)));
        timeline.MinSeekTime (winrt::Windows::Foundation::TimeSpan (0));
        timeline.MaxSeekTime (timeline.EndTime());
        s.UpdateTimelineProperties (timeline);

        if (! trackChanged) return;

        auto updater = s.DisplayUpdater();
        updater.Type (MediaPlaybackType::Music);
        updater.MusicProperties().Title (winrt::hstring (np.title.toWideCharPointer()));
        updater.MusicProperties().Artist (winrt::hstring (np.artist.toWideCharPointer()));
        updater.Update();

        if (! np.cover.isValid()) return;

        // the cover goes through a file: StorageFile is asynchronous, so open it off the message thread
        // (blocking on an async call is not allowed on the UI thread)
        const auto file = impl.coverFile.getSiblingFile ("Transport-cover-" + juce::String (++impl.coverCounter % 4) + ".png");
        file.deleteFile();
        {
            juce::FileOutputStream out (file);
            juce::PNGImageFormat png;
            if (! out.openedOk() || ! png.writeImageToStream (np.cover.rescaled (juce::jmin (512, np.cover.getWidth()), juce::jmin (512, np.cover.getHeight())), out))
                return;
        }
        const auto path = std::wstring (file.getFullPathName().toWideCharPointer());
        std::thread ([updater, path]
        {
            try
            {
                winrt::init_apartment (winrt::apartment_type::multi_threaded);
                auto storage = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync (path).get();
                updater.Thumbnail (winrt::Windows::Storage::Streams::RandomAccessStreamReference::CreateFromFile (storage));
                updater.Update();
            }
            catch (...) {}
        }).detach();
    }
    catch (...) {}
}

void destroy (std::unique_ptr<MediaControls::Impl>& impl)
{
    if (impl == nullptr) return;
    try
    {
        impl->smtc.ButtonPressed (impl->token);
        impl->smtc.IsEnabled (false);
    }
    catch (...) {}
    impl.reset();
}
} // namespace mediacontrols

#else   // Windows SDK without C++/WinRT: no overlay

struct MediaControls::Impl {};
namespace mediacontrols
{
std::unique_ptr<MediaControls::Impl> create (juce::Component&, MediaControls&) { return nullptr; }
void update (MediaControls::Impl&, const NowPlaying&, bool) {}
void destroy (std::unique_ptr<MediaControls::Impl>& p) { p.reset(); }
}
#endif
} // namespace tp
#endif
