#pragma once

#include <cstddef>
#include <cstdint>

namespace Netplay
{
constexpr std::size_t MAX_PLAYERS = 4;

enum class SessionPhase : std::uint8_t
{
    Hello = 0,
    Ready = 1,
};

struct SessionPacket
{
    std::uint64_t sessionId = 0;
    std::uint32_t seed = 0;
    std::uint32_t gameplayAbi = 0;
    std::uint32_t gameId = 0;
    std::uint8_t senderPlayer = 0;
    std::uint8_t playerCount = 0;
    SessionPhase phase = SessionPhase::Hello;
};
} // namespace Netplay
