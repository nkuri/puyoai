#include "capture/analyzer_result_drawer.h"

#include <SDL.h>
#include "capture/analyzer.h"
#include "capture/capture.h"
#include "core/real_color.h"
#include "gui/bounding_box.h"
#include "gui/color_map.h"
#include "gui/screen.h"
#include "gui/SDL_prims.h"

AnalyzerResultDrawer::AnalyzerResultDrawer(AnalyzerResultRetriever* retriever) :
    retriever_(retriever)
{
}

void AnalyzerResultDrawer::draw(Screen* screen)
{
    SDL_Surface* surface = screen->surface();
    if (!surface)
        return;

    unique_ptr<AnalyzerResult> result(retriever_->analyzerResult());
    if (!result.get())
        return;

    CHECK_EQ(SDL_LockSurface(surface), 0);

    for (int pi = 0; pi < 2; ++pi) {
        const PlayerAnalyzerResult* par = result->playerResult(pi);
        if (!par)
            continue;

        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                Box b = BoundingBox::instance().get(pi, x, y);
                RealColor rc = par->realColor(x, y);
                Uint32 color = ColorMap::instance().getColor(rc);
                SDL_DrawLine(surface, b.sx + 1, b.sy + 1, b.dx - 1, b.sy + 1, color);
                SDL_DrawLine(surface, b.sx + 1, b.sy + 1, b.sx + 1, b.dy - 1, color);
                SDL_DrawLine(surface, b.dx - 1, b.sy + 1, b.dx - 1, b.dy - 1, color);
                SDL_DrawLine(surface, b.sx + 1, b.dy - 1, b.dx - 1, b.dy - 1, color);
            }
        }
    }

    NextPuyoPosition npp[4] = {
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
    };

    for (int pi = 0; pi < 2; ++pi) {
        const PlayerAnalyzerResult* par = result->playerResult(pi);
        if (!par)
            continue;

        for (int i = 0; i < 4; ++i) {
            Box b = BoundingBox::instance().get(pi, npp[i]);
            RealColor rc = par->realColor(npp[i]);
            Uint32 color = ColorMap::instance().getColor(rc);
            SDL_DrawLine(surface, b.sx + 1, b.sy + 1, b.dx - 1, b.sy + 1, color);
            SDL_DrawLine(surface, b.sx + 1, b.sy + 1, b.sx + 1, b.dy - 1, color);
            SDL_DrawLine(surface, b.dx - 1, b.sy + 1, b.dx - 1, b.dy - 1, color);
            SDL_DrawLine(surface, b.sx + 1, b.dy - 1, b.dx - 1, b.dy - 1, color);
        }
    }

    SDL_UnlockSurface(surface);
}