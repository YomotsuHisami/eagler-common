#pragma once
#include <eagler/netplay/SpectatorStageProtocol.hpp>
#include <array>

namespace Netplay {
// Record once for EVERY executed frame, including resimulation. The title
// supplies only a scalar initializer captured before its first stage input and
// Boss registrations observed during this tick. Publication follows reconcile.
class SpectatorStagePublisher {
public:
    static constexpr unsigned Capacity = 256;
    struct Frame {
        std::uint32_t frame = INVALID_FRAME, stage = 0, schema = 0;
        std::vector<std::uint8_t> initializer;
        std::array<FrameInput, MAX_PLAYERS> inputs{};
        bool hasInputs = false;
        std::uint8_t bossSlots = 0;
    };
    bool Reset(std::uint8_t game, std::uint8_t count, std::uint64_t session, std::uint32_t abi,
               std::uint32_t firstFrame = 0) {
        SpectatorStageEvent check; check.game = game; check.playerCount = count;
        check.sessionId = session; check.gameplayAbi = abi; check.frame = firstFrame;
        check.serial = check.schema = 1; check.state = {1};
        if (!SpectatorStageWire::Valid(check)) return false;
        *this = {}; identity_ = check; identity_.state.clear(); identity_.schema = identity_.serial = 0;
        next_ = firstFrame;
        return true;
    }
    bool Record(Frame frame) {
        if (!identity_.sessionId || ended_ || frame.frame == INVALID_FRAME || frame.stage > 65535 ||
            frame.initializer.size() > SpectatorStageMaxStateBytes ||
            bool(frame.schema) != !frame.initializer.empty()) return false;
        if (frame.frame < next_) {
            // Restoring an older journal checkpoint may replay an already
            // confirmed prefix. It cannot amend published history. Retain one
            // ring of lanes to distinguish this from a real input revision.
            const auto& old = records_[frame.frame % Capacity];
            return old.frame != frame.frame || SameInputs(old, frame);
        }
        if (frame.frame - next_ >= Capacity) return false;
        if (frame.frame == next_ && phase_) {
            const auto& old = records_[frame.frame % Capacity];
            return old.frame == frame.frame && old.stage == frame.stage && old.schema == frame.schema &&
                old.initializer == frame.initializer && old.bossSlots == frame.bossSlots && SameInputs(old, frame);
        }
        records_[frame.frame % Capacity] = std::move(frame); return true;
    }
    template<class Inputs, class Send>
    bool Publish(std::uint32_t confirmed, std::uint32_t simulated, Inputs inputs, Send send) {
        if (!identity_.sessionId || ended_) return false;
        if (confirmed == INVALID_FRAME || simulated == INVALID_FRAME) return true;
        const auto last = std::min(confirmed, simulated);
        while (next_ <= last) {
            const auto& record = records_[next_ % Capacity];
            if (record.frame != next_) return false;
            if (phase_ == 0) {
                if (!record.initializer.empty()) {
                    auto event = identity_; event.frame = next_; event.stage = record.stage;
                    event.kind = SpectatorEventKind::Checkpoint; event.serial = serial_ + 1;
                    event.schema = record.schema; event.state = record.initializer;
                    if (!Event(event, send)) return false;
                    checkpoint_ = serial_; stage_ = record.stage;
                } else if (!checkpoint_ || record.stage != stage_) return false;
                phase_ = 1;
            }
            if (phase_ == 1) {
                SpectatorFramePacket packet; packet.sessionId = identity_.sessionId;
                packet.gameplayAbi = identity_.gameplayAbi; packet.playerCount = identity_.playerCount; packet.frame = next_;
                std::vector<std::uint8_t> wire;
                if (record.hasInputs) packet.inputs = record.inputs;
                else if (!inputs(next_, packet.inputs)) return false;
                if (!EncodeSpectatorFramePacket(packet, &wire) || !send(wire)) return false;
                phase_ = 2;
            }
            if (record.bossSlots) {
                auto event = identity_; event.kind = SpectatorEventKind::Node;
                event.frame = next_ + 1; event.stage = stage_; event.serial = serial_ + 1;
                event.checkpoint = checkpoint_; event.node = SpectatorNodeKind::Boss; event.bossSlots = record.bossSlots;
                if (!Event(event, send)) return false;
            }
            ++next_; phase_ = 0;
        }
        return true;
    }
    template<class Send> bool Finish(std::uint32_t endFrame, Send send) {
        if (ended_ || !checkpoint_ || phase_ || endFrame != next_) return false;
        auto event = identity_; event.kind = SpectatorEventKind::End; event.frame = next_;
        event.stage = stage_; event.serial = serial_ + 1;
        if (!Event(event, send)) return false;
        ended_ = true; return true;
    }
    std::uint32_t Published() const {return next_;}
    std::uint32_t Serial() const {return serial_;}
private:
    bool SameInputs(const Frame& a, const Frame& b) const {
        if (!a.hasInputs || !b.hasInputs) return true;
        SpectatorFramePacket packet; packet.sessionId=identity_.sessionId;
        packet.gameplayAbi=identity_.gameplayAbi; packet.playerCount=identity_.playerCount; packet.frame=a.frame;
        std::vector<std::uint8_t> left,right; packet.inputs=a.inputs;
        if (!EncodeSpectatorFramePacket(packet,&left)) return false;
        packet.inputs=b.inputs; return EncodeSpectatorFramePacket(packet,&right) && left==right;
    }
    template<class Send> bool Event(const SpectatorStageEvent& event, Send send) {
        if (!event.serial) return false;
        std::vector<std::uint8_t> wire;
        if (!EncodeSpectatorStageEvent(event, &wire) || !send(wire)) return false;
        serial_ = event.serial; return true;
    }
    std::array<Frame, Capacity> records_{};
    SpectatorStageEvent identity_;
    std::uint32_t next_ = 0, stage_ = 0, serial_ = 0, checkpoint_ = 0;
    unsigned phase_ = 0; bool ended_ = false;
};
}
