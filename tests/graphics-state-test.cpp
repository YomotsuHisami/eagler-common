#define TH_NATIVE_PLATFORM 1
#include <cassert>
#include <eagler/graphics/RenderCommands.hpp>
using namespace touhou::graphics;
struct Device : StateCommands {
    PipelineState state;
    PipelineState &pipeline() override { return state; }
};
int main() {
    Device device;
    RenderCommands<Device> commands(device);
    commands.SetDepthFunc(DepthFunc::Always);
    commands.SetDepthMask(false);
    commands.SetFogEnabled(true);
    commands.SetFogRange(10, 100);
    commands.SetColorOp(ColorOp::SelectFirst);
    commands.SetTextureArg(TextureArg::Factor);
    assert(device.state.depthCompare == Compare::Always && !device.state.depthWrite);
    assert(device.state.fog && device.state.fogNear == 10 && device.state.fogFar == 100);
    assert(device.state.color.operation == ColorOperation::First);
    assert(device.state.color.first.source == ArgumentSource::Factor);
    assert(device.state.alpha.first == device.state.color.first);
    auto other = device.state;
    assert(other.compatible(device.state));
    other.textureFactor = 0;
    assert(!other.compatible(device.state) && other.compatible(device.state, true));
    other.separateAlphaBlend = true;
    assert(!other.compatible(device.state, true));
    device.state.separateAlphaBlend = true;
    assert(other.compatible(device.state, true));
    other.destinationAlphaBlend = BlendFactor::InverseSourceAlpha;
    assert(!other.compatible(device.state, true));
}
