#include <eagler/netplay/NetplaySession.hpp>
#include <eagler/netplay/WebSocketTransport.hpp>

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace Netplay;

namespace
{
SessionPacket PeerPacket(const SessionConfig &config, std::uint8_t player, SessionPhase phase)
{
    SessionPacket packet;
    packet.sessionId = config.sessionId;
    packet.seed = config.seed;
    packet.gameplayAbi = config.gameplayAbi;
    packet.gameId = config.gameId;
    packet.senderPlayer = player;
    packet.playerCount = config.playerCount;
    packet.phase = phase;
    return packet;
}

void TestSessionGate()
{
    SessionConfig config;
    config.sessionId = 0x12345678u;
    config.seed = 0xabcdefu;
    config.gameplayAbi = 5;
    config.gameId = 7;
    config.playerCount = 3;
    config.localPlayer = 0;

    SessionGate gate;
    assert(gate.Reset(config));
    assert(!gate.CanSendReady());
    assert(!gate.CanStart());

    assert(gate.Apply(PeerPacket(config, 1, SessionPhase::Ready)) ==
           SessionPacketResult::ReadyBeforeHello);
    assert(gate.Apply(PeerPacket(config, 1, SessionPhase::Hello)) ==
           SessionPacketResult::Accepted);
    assert(gate.Apply(PeerPacket(config, 2, SessionPhase::Hello)) ==
           SessionPacketResult::Accepted);
    assert(gate.CanSendReady());

    gate.MarkLocalReady();
    assert(gate.LocalReady());
    assert(gate.Apply(PeerPacket(config, 1, SessionPhase::Ready)) ==
           SessionPacketResult::Accepted);
    assert(gate.Apply(PeerPacket(config, 2, SessionPhase::Ready)) ==
           SessionPacketResult::Accepted);
    assert(gate.CanStart());

    SessionPacket mismatch = PeerPacket(config, 1, SessionPhase::Hello);
    mismatch.gameplayAbi++;
    assert(gate.Apply(mismatch) == SessionPacketResult::ContractMismatch);
}

void TestNativeWebSocketStub()
{
#ifndef __EMSCRIPTEN__
    WebSocketTransport transport;
    assert(!transport.Connect("ws://127.0.0.1/unused"));
    assert(transport.Failed());
    assert(!transport.IsOpen());
    assert(transport.LastError() == "WebSocket transport is only implemented for Web/Emscripten");

    const std::uint8_t byte = 1;
    assert(!transport.Send(&byte, 1));
    std::vector<std::uint8_t> packet;
    assert(!transport.Poll(&packet));
    assert(transport.BufferedAmount() == 0);
#endif
}
} // namespace

int main()
{
    TestSessionGate();
    TestNativeWebSocketStub();
    std::cout << "eagler-common netplay base: PASS\n";
    return 0;
}
