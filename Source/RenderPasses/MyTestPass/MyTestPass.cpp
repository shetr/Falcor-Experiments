/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "MyTestPass.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, MyTestPass>();
}

MyTestPass::MyTestPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    mpProgram = Program::createGraphics(mpDevice, "RenderPasses/MyTestPass/MyTestShader.slang", "vsMain", "psMain");

    RasterizerState::Desc wireframeDesc;
    wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
    wireframeDesc.setCullMode(RasterizerState::CullMode::None);
    mpRasterState = RasterizerState::create(wireframeDesc);

    mpGraphicsState = GraphicsState::create(mpDevice);
    mpGraphicsState->setProgram(mpProgram);
    mpGraphicsState->setRasterizerState(mpRasterState);
}

Properties MyTestPass::getProperties() const
{
    return {};
}

RenderPassReflection MyTestPass::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addOutput("output", "Wireframe view texture");
    return reflector;
}

void MyTestPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pTargetFbo = Fbo::create(mpDevice, {renderData.getTexture("output")});
    Dictionary dict = renderData.getDictionary();
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

void MyTestPass::renderUI(Gui::Widgets& widget)
{}

void MyTestPass::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
    if (mpScene)
        mpProgram->addDefines(mpScene->getSceneDefines());
    mpVars = ProgramVars::create(mpDevice, mpProgram->getReflector());
}
