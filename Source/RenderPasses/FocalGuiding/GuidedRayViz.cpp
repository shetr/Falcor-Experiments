#include "GuidedRayViz.h"
#include "RenderGraph/RenderPassHelpers.h"
#include "RenderGraph/RenderPassStandardFlags.h"

#include "GuidedRayLine.h"

namespace
{
const char kShaderFile[] = "RenderPasses/FocalGuiding/GuidedRayViz.slang";

} // namespace

GuidedRayViz::GuidedRayViz(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    mpProgram = Program::createGraphics(mpDevice, kShaderFile, "vsMain", "psMain");
    RasterizerState::Desc wireframeDesc;
    wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    wireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpRasterState = RasterizerState::create(wireframeDesc);

    mpGraphicsState = GraphicsState::create(mpDevice);
    mpGraphicsState->setProgram(mpProgram);
    mpGraphicsState->setRasterizerState(mpRasterState);
}

Properties GuidedRayViz::getProperties() const
{
    return {};
}

RenderPassReflection GuidedRayViz::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    //reflector.addInput("input", "lines color");
    reflector.addOutput("output", "linesColor");
    return reflector;
}

void GuidedRayViz::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (!mpScene)
    {
        return;
    }

    Dictionary& dict = renderData.getDictionary();
    mGuidedRaysSize = dict["gGuidedRaysSize"];
    mGuidedRays = dict["gGuidedRays"];
    mComputeRays = dict["gComputeRays"];
    
    if (mComputeRays && mGuidedRays)
    {
        generateRaysGeometry();
    }

    auto pTargetFbo = Fbo::create(mpDevice, {renderData.getTexture("output")});
    const float4 clearColor(0, 0, 0, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    mpGraphicsState->setFbo(pTargetFbo);

    if (mpScene)
    {
        auto var = mpVars->getRootVar();
        var["PerFrameCB"]["gColor"] = float4(0, 1, 0, 1);

        mpScene->rasterize(pRenderContext, mpGraphicsState.get(), mpVars.get(), mpRasterState, mpRasterState);
    }
}

void GuidedRayViz::renderUI(Gui::Widgets& widget)
{
    
}

void GuidedRayViz::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;

    if (mpScene)
        mpProgram->addDefines(mpScene->getSceneDefines());
    mpVars = ProgramVars::create(mpDevice, mpProgram->getReflector());
}

void GuidedRayViz::prepareVars()
{
    // mNodes =
    //     mpDevice->createBuffer(mNodesSize * sizeof(DensityNode), bindFlags | ResourceBindFlags::Shared, memoryType, densityNodes.data());
    // mpDevice->createBuffer()

    //SceneBuilder builder();
    //SceneBuilder pBuilder = SceneBuilder::create();
    //
    //SceneBuilder::Mesh mesh;
    //// ... Fill out mesh struct ...
    //size_t meshId = pBuilder->addMesh(mesh);
    //size_t nodeId = pBuilder->addNode(Node(/* Instance matrix */));
    //pBuilder->addMeshInstance(nodeId, meshId);
    //
    //Scene::SharedPtr pScene = pBuilder->getScene();
}

void GuidedRayViz::generateRaysGeometry()
{
    std::vector<GuidedRayLine> rayNodes = mGuidedRays->getElements<GuidedRayLine>(0, mGuidedRaysSize);

    for (uint i = 0; i < mGuidedRaysSize; ++i)
    {
        printf("ray: %d\n", i);
        GuidedRayLine rayLine = rayNodes[i];
        printf("pos1: %f, %f, %f\n", rayLine.pos1.x, rayLine.pos1.y, rayLine.pos1.z);
        printf("pos2: %f, %f, %f\n", rayLine.pos2.x, rayLine.pos2.y, rayLine.pos2.z);
        printf("color: %f, %f, %f\n", rayLine.color.x, rayLine.color.y, rayLine.color.z);
    }
}
