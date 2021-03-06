#include "capture/capture.h"

#include "capture/source.h"
#include "gui/screen.h"
#include "gui/SDL_prims.h"

using namespace std;

Capture::Capture(Source* source, Analyzer* analyzer) :
    source_(source),
    analyzer_(analyzer),
    shouldStop_(false),
    surface_(makeUniqueSDLSurface(nullptr))
{
}

bool Capture::start()
{
    th_ = thread([this](){
        this->runLoop();
    });
    th_.detach();
    return true;
}

void Capture::stop()
{
    shouldStop_ = true;
    if (th_.joinable())
        th_.join();
}

void Capture::runLoop()
{
    int frameId = 0;
    UniqueSDLSurface prevSurface(emptyUniqueSDLSurface());

    while (!shouldStop_) {
        UniqueSDLSurface surface(source_->getNextFrame());
        if (!surface.get())
            continue;

        // We set frameId to surface's userdata. This will be useful for saving screen shot.
        surface->userdata = reinterpret_cast<void*>(static_cast<uintptr_t>(++frameId));

        lock_guard<mutex> lock(mu_);
        unique_ptr<AnalyzerResult> r = analyzer_->analyze(surface.get(), prevSurface.get(), results_);
        prevSurface = move(surface_);
        surface_ = move(surface);
        results_.push_front(move(r));
        while (results_.size() > 10)
            results_.pop_back();
    }
}

void Capture::draw(Screen* screen)
{
    SDL_Surface* surface = screen->surface();

    lock_guard<mutex> lock(mu_);
    SDL_BlitSurface(surface_.get(), nullptr, surface, nullptr);
}

unique_ptr<AnalyzerResult> Capture::analyzerResult() const
{
    lock_guard<mutex> lock(mu_);

    if (results_.empty())
        return unique_ptr<AnalyzerResult>();

    return results_.front().get()->copy();
}
