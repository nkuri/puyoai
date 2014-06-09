#include "wii/wii_connect_server.h"

#include <vector>

#include "capture/analyzer.h"
#include "capture/source.h"
#include "core/ctrl.h"
#include "core/puyo_color.h"
#include "core/server/connector/connector_manager_linux.h"
#include "gui/screen.h"
#include "wii/key_sender.h"

#include <iostream>

using namespace std;

WiiConnectServer::WiiConnectServer(Source* source, Analyzer* analyzer, KeySender* keySender,
                                   const string& p1Program, const string& p2Program) :
    shouldStop_(false),
    surface_(emptyUniqueSDLSurface()),
    source_(source),
    analyzer_(analyzer),
    keySender_(keySender)
{
    vector<string> programs {
        p1Program,
        p2Program,
    };

    isAi_[0] = (p1Program != "-");
    isAi_[1] = (p2Program != "-");

    connector_.reset(new ConnectorManagerLinux(programs));
    connector_->setWaitTimeout(false);
}

WiiConnectServer::~WiiConnectServer()
{
}

bool WiiConnectServer::start()
{
    return pthread_create(&th_, nullptr, runLoopCallback, this) == 0;
}

void WiiConnectServer::stop()
{
    shouldStop_ = true;
}

// static
void* WiiConnectServer::runLoopCallback(void* p)
{
    reinterpret_cast<WiiConnectServer*>(p)->runLoop();
    return nullptr;
}

void WiiConnectServer::reset()
{
    for (int i = 0; i < 2; ++i) {
        lastDecision_[i] = Decision();
        estimatedKeySec_[i] = 0;
        for (int j = 0; j < 6; ++j)
            invisiblePuyoColors_[i][j] = PuyoColor::EMPTY;
        currentPuyo_[i] = Kumipuyo();
    }

    colorMap_.clear();
    colorMap_.insert(make_pair(RealColor::RC_EMPTY, PuyoColor::EMPTY));
    colorMap_.insert(make_pair(RealColor::RC_OJAMA, PuyoColor::OJAMA));
    colorsUsed_.fill(false);
}

void WiiConnectServer::runLoop()
{
    int frameId = 0;
    while (!shouldStop_) {
        UniqueSDLSurface surface(source_->getNextFrame());
        if (!surface.get())
            continue;

        unique_ptr<AnalyzerResult> r = analyzer_->analyze(surface.get(), analyzerResults_);
        LOG(INFO) << r->toString();

        switch (r->state()) {
        case CaptureGameState::UNKNOWN:
            if (!playForUnknown(frameId))
                shouldStop_ = true;
            break;
        case CaptureGameState::LEVEL_SELECT:
            // TODO(mayah): For workaround, we make frameId = 1.
            // Server should send some event to initialize a game state.
            // Client should implement an initialization logic
            {
                // TODO(mayah): initialization should be done after NEXT1/NEXT2 are stabilized?
                ScopedLock lock(&mu_);
                if (!analyzerResults_.empty() && analyzerResults_.front()->state() != CaptureGameState::LEVEL_SELECT) {
                    frameId = 1;
                    reset();
                }
            }
            if (!playForLevelSelect(frameId, *r))
                shouldStop_ = true;
            break;
        case CaptureGameState::PLAYING:
            if (!playForPlaying(frameId, *r))
                shouldStop_ = true;
            break;
        case CaptureGameState::FINISHED:
            if (!playForFinished(frameId))
                shouldStop_ = true;
            break;
        }

        // We set frameId to surface's userdata. This will be useful for saving screen shot.
        surface->userdata = reinterpret_cast<void*>(static_cast<uintptr_t>(frameId));

        {
            ScopedLock lock(&mu_);
            surface_ = move(surface);
            analyzerResults_.push_front(move(r));
            while (analyzerResults_.size() > 10)
                analyzerResults_.pop_back();
        }

        frameId++;
    }
}

bool WiiConnectServer::playForUnknown(int frameId)
{
    if (frameId % 10 == 0)
        keySender_->sendKey(KEY_NONE);

    reset();
    return true;
}

bool WiiConnectServer::playForLevelSelect(int frameId, const AnalyzerResult& analyzerResult)
{
    if (frameId % 10 == 0) {
        keySender_->sendKey(KEY_NONE);
        keySender_->sendKey(KEY_RIGHT_TURN);
        keySender_->sendKey(KEY_NONE);
    }

    // Sends an initialization message.
    for (int pi = 0; pi < 2; pi++) {
        if (!connector_->IsConnectorAlive(pi)) {
            fprintf(stderr, "player #%d was disconnected\n", pi);
            return false;
        }

        if (isAi_[pi])
            connector_->Write(pi, makeMessageFor(pi, frameId, analyzerResult));
    }

    return true;
}

bool WiiConnectServer::playForPlaying(int frameId, const AnalyzerResult& analyzerResult)
{
    for (int pi = 0; pi < 2; pi++) {
        if (!connector_->IsConnectorAlive(pi)) {
            fprintf(stderr, "player #%d was disconnected\n", pi);
            return false;
        }

        if (isAi_[pi])
            connector_->Write(pi, makeMessageFor(pi, frameId, analyzerResult));
    }

    vector<PlayerLog> all_data;
    connector_->GetActions(frameId, &all_data);

    for (int pi = 0; pi < 2; pi++) {
        if (!isAi_[pi])
            continue;

        if (analyzerResult.playerResult(pi)->userState.grounded) {
            lastDecision_[pi] = Decision();
            keySender_->sendKey(KEY_NONE);
        }

        const PlayerLog& data = all_data[pi];
        outputKeys(pi, analyzerResult, data);
    }

    return true;
}

bool WiiConnectServer::playForFinished(int frameId)
{
    if (frameId % 10 == 0) {
        keySender_->sendKey(KEY_NONE);
        keySender_->sendKey(KEY_START);
        keySender_->sendKey(KEY_NONE);
    }

    reset();
    return true;
}

string WiiConnectServer::makeStateString(int playerId, const AnalyzerResult& re)
{
    int s1 = 0, s2 = 0;

    s1 |= re.playerResult(0)->userState.playable      ? STATE_YOU_CAN_PLAY   : 0;
    s1 |= re.playerResult(0)->userState.wnextAppeared ? STATE_WNEXT_APPEARED : 0;
    s1 |= re.playerResult(0)->userState.grounded      ? STATE_YOU_GROUNDED   : 0;
    s1 |= re.playerResult(0)->userState.chainFinished ? STATE_CHAIN_DONE     : 0;
    s1 |= re.playerResult(0)->userState.ojamaDropped  ? STATE_OJAMA_DROPPED  : 0;

    s2 |= re.playerResult(1)->userState.playable      ? STATE_YOU_CAN_PLAY   : 0;
    s2 |= re.playerResult(1)->userState.wnextAppeared ? STATE_WNEXT_APPEARED : 0;
    s2 |= re.playerResult(1)->userState.grounded      ? STATE_YOU_GROUNDED   : 0;
    s2 |= re.playerResult(1)->userState.chainFinished ? STATE_CHAIN_DONE     : 0;
    s2 |= re.playerResult(1)->userState.ojamaDropped  ? STATE_OJAMA_DROPPED  : 0;

    int s = playerId == 0 ? (s1 | (s2 << 1)) : (s2 | (s1 << 1));

    return to_string(s);
}

string WiiConnectServer::makeMessageFor(int playerId, int frameId, const AnalyzerResult& re)
{
    CoreField field[2];
    PuyoColor nexts[2][NUM_NEXT_PUYO_POSITION];

    // First, fill the current field and nexts.
    // We only allocate a new puyo color when it appears in the next-puyo field.
    for (int pi = 0; pi < 2; ++pi) {
        const PlayerAnalyzerResult* pr = re.playerResult(pi);
        for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i)
            nexts[pi][i] = toPuyoColor(pr->adjustedField.nextPuyos[i], true);

        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                field[pi].unsafeSet(x, y, toPuyoColor(pr->adjustedField.puyos[x-1][y-1]));
            }
            field[pi].recalcHeightOn(x);
        }
    }

    const CoreField& myField = field[playerId];
    const CoreField& enemyField = field[1 - playerId];
    PuyoColor* myNexts = nexts[playerId];
    PuyoColor* enemyNexts = nexts[1 - playerId];

    // Then, send message.
    stringstream ss;
    ss << "ID=" << frameId
       << " STATE=" << makeStateString(playerId, re);
    ss << " YF=";
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << static_cast<int>(myField.get(x, y));
        }
    }
    ss << " YP=";
    for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i) {
        ss << static_cast<int>(myNexts[i]);
    }
    ss << " OF=";
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << static_cast<int>(enemyField.get(x, y));
        }
    }
    ss << " OP=";
    for (int i = 0; i < NUM_NEXT_PUYO_POSITION; ++i) {
        ss << static_cast<int>(enemyNexts[i]);
    }
    ss << " YX=3 YY=12 YR=0 OX=3 OY=12 OR=0";
    ss << " YO=0 OO=0 YS=0 OS=0";
    ss << "\n";

    return ss.str();
}

PuyoColor WiiConnectServer::toPuyoColor(RealColor rc, bool allowAllocation)
{
    auto it = colorMap_.find(rc);
    if (it != colorMap_.end())
        return it->second;

    if (!allowAllocation) {
        LOG(WARNING) << toString(rc) << " cannot mapped to be a puyo color. EMPTY is returned instead.";
        return PuyoColor::EMPTY;
    }

    DCHECK(isNormalColor(rc)) << toString(rc);
    for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
        if (colorsUsed_[i])
            continue;

        colorsUsed_[i] = true;
        PuyoColor pc = normalPuyoColorOf(i);
        colorMap_.insert(make_pair(rc, pc));

        LOG(INFO) << "Detected a new color: " << toString(rc) << "->" << toString(pc);
        return pc;
    }

    // 5th color comes... :-(
    LOG(ERROR) << "Detected 5th color: " << toString(rc);
    return PuyoColor::EMPTY;
}

void WiiConnectServer::outputKeys(int pi, const AnalyzerResult& analyzerResult, const PlayerLog& data)
{
    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = data.received_data.size(); i > 0; ) {
        i--;

        const Decision& d = data.received_data[i].decision;

        // We don't need ACK/NACK for ID only messages.
        if (!d.isValid() || d == lastDecision_[pi])
            continue;

        // TODO(mayah): Set an approapriate value for Y.
        KumipuyoPos start = lastDecision_[pi].isValid() ? KumipuyoPos(lastDecision_[pi].x, 12, lastDecision_[pi].r) : KumipuyoPos(3, 12, 0);
        // TODO(mayah): It's OK since Ctrl::getControlOnline doesn't use Y.
        KumipuyoPos goal(d.x, 1, d.r);

        // ----------
        // Set's the current area.
        // This is for checking the destination is reachable, it is ok to set ojama if the cell is occupied.
        vector<KeyTuple> keys;
        {
            CoreField field;
            const AdjustedField& af = analyzerResult.playerResult(pi)->adjustedField;
            for (int x = 1; x <= 6; ++x) {
                for (int y = 1; y <= 12; ++y) {
                    if (af.puyos[x-1][y-1] == RealColor::RC_EMPTY)
                        break;

                    field.unsafeSet(x, y, PuyoColor::OJAMA);
                }
                field.recalcHeightOn(x);
            }

            // Not controllable?
            if (!Ctrl::getControlOnline(field, goal, start, &keys))
                continue;
        }

        lastDecision_[pi] = d;
        for (size_t j = 0; j < keys.size(); j++) {
            KeyTuple k = keys[j];
            if (k.b2 != KEY_NONE) {
                keySender_->sendKey(k.b1, k.b2);
            } else {
                keySender_->sendKey(k.b1);
            }

            if (j + 1 < keys.size() && k.hasSameKey(keys[j+1])) {
                keySender_->sendKey(KEY_NONE);
            }
        }

        return;
    }
}

void WiiConnectServer::draw(Screen* screen)
{
    SDL_Surface* surface = screen->surface();
    if (!surface)
        return;

    ScopedLock lock(&mu_);

    if (!surface_.get())
        return;

    surface->userdata = surface_->userdata;
    SDL_BlitSurface(surface_.get(), nullptr, surface, nullptr);
}

unique_ptr<AnalyzerResult> WiiConnectServer::analyzerResult() const
{
    ScopedLock lock(&mu_);

    if (analyzerResults_.empty())
        return unique_ptr<AnalyzerResult>();

    return analyzerResults_.front().get()->copy();
}