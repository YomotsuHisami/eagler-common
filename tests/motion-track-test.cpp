#include <cassert>
#include <eagler/input/MotionTrack.hpp>
#include <fstream>
#include <iterator>
#include <string>
using Track = eagler::input::MotionTrack<>;
static std::vector<std::uint8_t> read(const std::string &path) {
    std::ifstream file(path, std::ios::binary);
    assert(file);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
static void checksum(std::vector<std::uint8_t> &bytes) {
    const auto length = bytes.size() - 24;
    Track::put_word(bytes.data() + length + 20, Track::hash(bytes.data(), length));
}
int main(int argc, char **argv) {
    assert(argc == 2);
    const auto v1 = read(std::string(argv[1]) + "/motion-v1.bin");
    const auto v2 = read(std::string(argv[1]) + "/motion-v2.bin");
    Track track;
    eagler::input::MotionTrack<false> movementOnly;
    assert(track.load(v1.data(), v1.size(), 8));
    assert(track.trailer(8) == v1);
    assert(movementOnly.load(v1.data(), v1.size(), 8));
    assert(movementOnly.trailer(8) == v1);
    assert(!movementOnly.load(v2.data(), v2.size(), 8));
    assert(track.load(v2.data(), v2.size(), 8));
    assert(track.trailer(8) == v2);
    track.begin(0, true, true, false);
    float x = 0, y = 0;
    Track::TouchPoint point;
    assert(track.playback(0, x, y) && x == 1.25f && y == -2.5f);
    assert(track.replay_points(&point, 1) == 1 && point.id == -17 && point.x == 4.5f &&
           !point.released);
    assert(!track.playback(0, x, y));
    assert(track.replay_points(&point, 1) == 1 && point.x == 5.0f);
    assert(track.playback(0, x, y) && std::signbit(x) && y == 3.75f);
    assert(track.replay_points(&point, 1) == 1 && point.released);
    assert(!track.playback(0, x, y) && track.replay_points(&point, 1) == 0);
    assert(!track.load(v1.data(), v1.size(), 15) && !track.used());
    auto invalid = v1;
    invalid[36 + 4] ^= 1; // Corrupted payload.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    invalid = v1;
    Track::put_word(invalid.data() + 48, 0);
    checksum(invalid); // Duplicate movement tick.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    invalid = v1;
    Track::put_word(invalid.data() + 40, 0x7fc00000);
    checksum(invalid); // NaN, valid checksum.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    invalid = v1;
    Track::put_word(invalid.data(), 0xffffffff);
    checksum(invalid); // Overflowing entry count.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    invalid = v2;
    Track::put_word(invalid.data() + 108 + 16, 4);
    checksum(invalid); // Unknown touch action.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    invalid = v1;
    invalid.erase(invalid.begin()); // Footer retained; declared payload exceeds input.
    assert(!track.load(invalid.data(), invalid.size(), 8));
    const std::uint8_t originalReplay[]{0x54, 0x38, 0x52, 0x50};
    assert(track.load(originalReplay, sizeof(originalReplay), 8) && !track.used());
    track.begin(0, true, false, true);
    track.target(2, 10, 20);
    track.mark_cheat_movement(1, 0);
    track.record(0, true, 2, 3);
    track.record(0, false, 0, 0);
    track.touch_event(0, 0, -1, 4, 5);
    track.record(0, true, -1, 2);
    auto recorded = track.trailer(8);
    Track replay;
    assert(replay.load(recorded.data(), recorded.size(), 8));
    assert(replay.stages[0].size() == 2 && replay.stages[0][1].tick == 2);
    assert(replay.touch_stages[0][0].id == -1 && track.cheat_movement_used);
    track.record(0, true, std::numeric_limits<float>::infinity(), 0);
    assert(track.invalid && track.trailer(8).empty());
}
