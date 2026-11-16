// macOS "Now Playing" (menu bar / Control Centre) and the media keys, through the MediaPlayer framework.
#include "MediaControls.h"
#import <MediaPlayer/MediaPlayer.h>
#import <AppKit/AppKit.h>

namespace tp
{
struct MediaControls::Impl
{
    MediaControls* owner = nullptr;
    NSMutableArray* targets = nil;   // (command, target) pairs to remove again
};

namespace mediacontrols
{
std::unique_ptr<MediaControls::Impl> create (juce::Component&, MediaControls& owner)
{
    if (@available (macOS 10.12.2, *))
    {
        auto impl = std::make_unique<MediaControls::Impl>();
        impl->owner = &owner;
        impl->targets = [NSMutableArray array];
        MPRemoteCommandCenter* cc = [MPRemoteCommandCenter sharedCommandCenter];
        auto* o = &owner;

        auto hook = [&] (MPRemoteCommand* command, std::function<void()> MediaControls::* member)
        {
            id target = [command addTargetWithHandler: ^MPRemoteCommandHandlerStatus (MPRemoteCommandEvent*)
            {
                juce::MessageManager::callAsync ([o, member] { if (o->*member) (o->*member)(); });
                return MPRemoteCommandHandlerStatusSuccess;
            }];
            command.enabled = YES;
            [impl->targets addObject: @[ command, target ]];
        };
        hook (cc.togglePlayPauseCommand, &MediaControls::onPlayPause);
        hook (cc.playCommand,            &MediaControls::onPlay);
        hook (cc.pauseCommand,           &MediaControls::onPause);
        hook (cc.nextTrackCommand,       &MediaControls::onNext);
        hook (cc.previousTrackCommand,   &MediaControls::onPrev);
        return impl;
    }
    return nullptr;
}

void update (MediaControls::Impl&, const NowPlaying& np, bool trackChanged)
{
    if (@available (macOS 10.12.2, *))
    {
        MPNowPlayingInfoCenter* center = [MPNowPlayingInfoCenter defaultCenter];
        NSMutableDictionary* info = center.nowPlayingInfo != nil && ! trackChanged ? [center.nowPlayingInfo mutableCopy]
                                                                                    : [NSMutableDictionary dictionary];
        info[MPMediaItemPropertyTitle] = [NSString stringWithUTF8String: np.title.toRawUTF8()];
        info[MPMediaItemPropertyArtist] = [NSString stringWithUTF8String: np.artist.toRawUTF8()];
        info[MPMediaItemPropertyPlaybackDuration] = @(np.length);
        info[MPNowPlayingInfoPropertyElapsedPlaybackTime] = @(np.position);
        info[MPNowPlayingInfoPropertyPlaybackRate] = @(np.playing ? 1.0 : 0.0);

        if (trackChanged && np.cover.isValid())
        {
            juce::MemoryOutputStream png;
            juce::PNGImageFormat().writeImageToStream (np.cover, png);
            NSData* data = [NSData dataWithBytes: png.getData() length: png.getDataSize()];
            NSImage* image = [[NSImage alloc] initWithData: data];
            if (image != nil)
                info[MPMediaItemPropertyArtwork] = [[MPMediaItemArtwork alloc] initWithBoundsSize: image.size
                                                                                   requestHandler: ^NSImage* (CGSize) { return image; }];
        }
        center.nowPlayingInfo = info;
        center.playbackState = ! np.hasTrack ? MPNowPlayingPlaybackStateStopped
                                             : (np.playing ? MPNowPlayingPlaybackStatePlaying : MPNowPlayingPlaybackStatePaused);
    }
}

void destroy (std::unique_ptr<MediaControls::Impl>& impl)
{
    if (impl == nullptr) return;
    for (NSArray* pair in impl->targets)
        [(MPRemoteCommand*) pair[0] removeTarget: pair[1]];
    if (@available (macOS 10.12.2, *))
        [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
    impl.reset();
}
} // namespace mediacontrols
} // namespace tp
