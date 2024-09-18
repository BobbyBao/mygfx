/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// No include guard!!!

/*
 * How to use this header file.
 *
 * The DECL_DRIVER_API() macro must be defined before including this file:
 *     DECL_DRIVER_API(method_name, params_decl, params)
 *
 *     method_name          the name of the method (e.g. beginRenderPass)
 *     params_decl          the list of typed params
 *     params               the parameters, without their type (e.g.: for a function call)
 *
 * The following example adds all the driver methods as virtual
 * members of class:
 *
 * class MyDriver : public Driver {
 * public:
 * #define DRIVER_API(method_name, params_decl, params) \
 *     virtual void method_name(params_decl) override
 * #include "DriverAPI.h"
 * };
 *
 * DECL_DRIVER_API is automatically undefined.
 */

#ifndef DECL_DRIVER_API
#error "The DECL_DRIVER_API() macro must be defined before including this file"
#define DECL_DRIVER_API(M, D, P)
#endif

#ifndef DECL_DRIVER_API_SYNCHRONOUS
#error "The DECL_DRIVER_API_SYNCHRONOUS() macro must be defined before including this file"
#define DECL_DRIVER_API_SYNCHRONOUS(R, M, D, P)
#endif

#ifndef DECL_DRIVER_API_RETURN
#error "The DECL_DRIVER_API_RETURN() macro must be defined before including this file"
#define DECL_DRIVER_API_RETURN(R, M, D, P)
#endif


/*
 * Convenience macros. These are PRIVATE, don't use.
 */

#define EXPAND(x) x

#define APPLY0(M,...)
#define APPLY1(M, A, ...) EXPAND(M(A))
#define APPLY2(M, A, ...) EXPAND(M(A)), EXPAND(APPLY1(M, __VA_ARGS__))
#define APPLY3(M, A, ...) EXPAND(M(A)), EXPAND(APPLY2(M, __VA_ARGS__))
#define APPLY4(M, A, ...) EXPAND(M(A)), EXPAND(APPLY3(M, __VA_ARGS__))
#define APPLY5(M, A, ...) EXPAND(M(A)), EXPAND(APPLY4(M, __VA_ARGS__))
#define APPLY6(M, A, ...) EXPAND(M(A)), EXPAND(APPLY5(M, __VA_ARGS__))
#define APPLY7(M, A, ...) EXPAND(M(A)), EXPAND(APPLY6(M, __VA_ARGS__))
#define APPLY8(M, A, ...) EXPAND(M(A)), EXPAND(APPLY7(M, __VA_ARGS__))
#define APPLY9(M, A, ...) EXPAND(M(A)), EXPAND(APPLY8(M, __VA_ARGS__))
#define APPLY10(M, A, ...) EXPAND(M(A)), EXPAND(APPLY9(M, __VA_ARGS__))
#define APPLY11(M, A, ...) EXPAND(M(A)), EXPAND(APPLY10(M, __VA_ARGS__))
#define APPLY12(M, A, ...) EXPAND(M(A)), EXPAND(APPLY11(M, __VA_ARGS__))
#define APPLY_N__(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, X, ...) APPLY##X
#define APPLY(M, ...) EXPAND(EXPAND(APPLY_N__(M, __VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))(M, __VA_ARGS__))


#define PAIR_ARGS_0(M, ...)
#define PAIR_ARGS_1(M, X, Y, ...) M(X, Y)
#define PAIR_ARGS_2(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_1(M, __VA_ARGS__))
#define PAIR_ARGS_3(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_2(M, __VA_ARGS__))
#define PAIR_ARGS_4(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_3(M, __VA_ARGS__))
#define PAIR_ARGS_5(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_4(M, __VA_ARGS__))
#define PAIR_ARGS_6(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_5(M, __VA_ARGS__))
#define PAIR_ARGS_7(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_6(M, __VA_ARGS__))
#define PAIR_ARGS_8(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_7(M, __VA_ARGS__))
#define PAIR_ARGS_9(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_8(M, __VA_ARGS__))
#define PAIR_ARGS_10(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_9(M, __VA_ARGS__))
#define PAIR_ARGS_11(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_10(M, __VA_ARGS__))
#define PAIR_ARGS_12(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_11(M, __VA_ARGS__))

#define PAIR_ARGS_N__(_0, E1, _1, E2, _2, E3, _3, E4, _4, E5, _5, E6, _6, E7, _7, E8, _8, E9, _9, E10, _10, E11, _11, E12, _12, X, ...) PAIR_ARGS_##X

#define PAIR_ARGS_N(M, ...) \
    EXPAND(EXPAND(PAIR_ARGS_N__(0, ##__VA_ARGS__, 12, E, 11, E, 10, E, 9, E, 8, E, 7, E, 6, E, 5, E, 4, E, 3, E, 2, E, 1, E, 0))(M, __VA_ARGS__))

#define ARG(T, P) T P

#define PARAM(T, P) P

#define DECL_DRIVER_API_N(N, ...) \
    DECL_DRIVER_API(N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

#define DECL_DRIVER_API_R_N(R, N, ...) \
    DECL_DRIVER_API_RETURN(R, N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

#define DECL_DRIVER_API_SYNCHRONOUS_N(R, N, ...) \
    DECL_DRIVER_API_SYNCHRONOUS(R, N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

// on some compilers the ##__VA_ARGS__ hack is not supported, so we can't handle 0-parameter APIs
// with DECL_DRIVER_API_SYNCHRONOUS_N
#define DECL_DRIVER_API_SYNCHRONOUS_0(R, N) \
    DECL_DRIVER_API_SYNCHRONOUS(R, N,,)

// we can't handle empty lists because we don't have __VA_OPT__() in C++14 and the ## hack
// only works for omitted arguments, not empty lists
#define PARAM_1( P, ...) P
#define ARG_1( T, P, ...) T P
#define DECL_DRIVER_API_0(N)      DECL_DRIVER_API(N, ARG_1(int, dummy=0), PARAM_1(dummy))
#define DECL_DRIVER_API_R_0(R, N) DECL_DRIVER_API_RETURN(R, N, ARG_1(int, dummy=0), PARAM_1(dummy))

/*
 * Driver API below...
 */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#endif

/*
 * Synchronous APIs
 * ----------------
 */
 
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwSwapchain>, createSwapchain, const SwapChainDesc&, desc)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwRenderTarget>, createRenderTarget, const RenderTargetDesc&, desc)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwBuffer>, createBuffer, BufferUsage, usage, MemoryUsage, memoryUsage, uint64_t, size, uint16_t, stride, const void*, data)
DECL_DRIVER_API_SYNCHRONOUS_N(SharedPtr<HwTexture>, createTexture, const TextureData&, textureData, SamplerInfo, sampler)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwTextureView>, createSRV, HwTexture*, tex, int, mipLevel, const char*, name)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwTextureView>, createRTV, HwTexture*, tex, int, mipLevel, const char*, name)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, copyData, HwTexture*, tex, TextureDataProvider*, dataProvider)
DECL_DRIVER_API_SYNCHRONOUS_N(SamplerHandle, createSampler, const SamplerInfo&, info)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwShaderModule>, createShaderModule, ShaderStage, stage, const ByteArray&, shaderCode, ShaderCodeType, shaderCodeType, const char*, pShaderEntryPoint)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwProgram>, createProgram, Ref<HwShaderModule>*, shaderModules, uint32_t, count)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwVertexInput>, createVertexInput, const FormatList&, fmts, const FormatList&, fmts1)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwRenderPrimitive>, createRenderPrimitive, VertexData*, geo, const DrawPrimitiveCommand&, primitive)
DECL_DRIVER_API_SYNCHRONOUS_N(Ref<HwDescriptorSet>, createDescriptorSet, const Span<DescriptorSetLayoutBinding>&, bindings)

DECL_DRIVER_API_SYNCHRONOUS_N(bool, allocVertexBuffer, uint32_t, numbeOfVertices, uint32_t, strideInBytes, void**, pData, BufferInfo*, pOut)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, allocIndexBuffer, uint32_t, numbeOfIndices, uint32_t, strideInBytes, void**, pData, BufferInfo*, pOut)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, allocConstantBuffer, uint32_t, size, void**, pData, BufferInfo*, pOut)

DECL_DRIVER_API_SYNCHRONOUS_N(void, updateBuffer, 
    HwBuffer*, buffer,
    const void*, data,
    size_t, size,
    size_t, offset)

DECL_DRIVER_API_SYNCHRONOUS_N(void, updateTexture,
    HwTexture*, texture,
    uint32_t, level,
    uint32_t, xoffset,
    uint32_t, yoffset,
    uint32_t, zoffset,
    uint32_t, width,
    uint32_t, height,
    uint32_t, depth,
    const void*, data, size_t, size)
    
DECL_DRIVER_API_SYNCHRONOUS_N(void, copyTexture,
    HwTexture*, srcTex, uint32_t, srcLevel, uint32_t, srcLayer,
    HwTexture*, destTex, uint32_t, destLevel, uint32_t, destLayer)

/*
 * Rendering operations
 * --------------------
 */
 
DECL_DRIVER_API_0(beginFrame)
DECL_DRIVER_API_0(endFrame)

DECL_DRIVER_API_N(makeCurrent, HwSwapchain*, sc)
DECL_DRIVER_API_N(resize, HwSwapchain*, sc, uint32_t, destWidth, uint32_t, destHeight)
DECL_DRIVER_API_N(commit, HwSwapchain*, sc)

DECL_DRIVER_API_N(updateDescriptorSet1, HwDescriptorSet*, descriptorSet, uint32_t, dstBinding, HwTextureView*, textureView)
DECL_DRIVER_API_N(updateDescriptorSet2, HwDescriptorSet*, descriptorSet, uint32_t, dstBinding, HwBuffer*, buffer)
DECL_DRIVER_API_N(updateDescriptorSet3, HwDescriptorSet*, descriptorSet, uint32_t, dstBinding, const BufferInfo&, bufferInfo)

DECL_DRIVER_API_N(beginRendering, HwRenderTarget*, rt, const RenderPassInfo&, renderInfo)
DECL_DRIVER_API_N(endRendering, HwRenderTarget*, rt)

DECL_DRIVER_API_N(setViewport, float, topX, float, topY, float, width, float, height, float, minDepth, float, maxDepth)
DECL_DRIVER_API_N(setScissor, uint32_t, topX, uint32_t, topY, uint32_t, width, uint32_t, height)
DECL_DRIVER_API_N(setViewportAndScissor, uint32_t, topX, uint32_t, topY, uint32_t, width, uint32_t, height)
DECL_DRIVER_API_0(resetState)
DECL_DRIVER_API_N(setVertexInput, HwVertexInput*, vertexInput)
DECL_DRIVER_API_N(setPrimitiveTopology, PrimitiveTopology, primitiveTopology)
DECL_DRIVER_API_N(setPrimitiveRestartEnable, bool, restartEnable)

DECL_DRIVER_API_N(bindShaderProgram, HwProgram*, program)
DECL_DRIVER_API_N(bindRasterState, RasterState*, rasterState)
DECL_DRIVER_API_N(bindColorBlendState, ColorBlendState*, colorBlendState)
DECL_DRIVER_API_N(bindDepthState, DepthState*, depthState)
DECL_DRIVER_API_N(bindStencilState, StencilState*, stencilState)
DECL_DRIVER_API_N(bindPipelineState, const PipelineState&, pipelineState)
DECL_DRIVER_API_N(pushConstant, uint32_t, index, const void*, data, uint32_t, size)
DECL_DRIVER_API_N(bindUniforms, const Uniforms&, uniforms)
DECL_DRIVER_API_N(bindIndexBuffer, HwBuffer*, buffer, uint64_t, offset, IndexType, indexType)
DECL_DRIVER_API_N(bindVertexBuffer, uint32_t, firstBinding, HwBuffer*, buffer, uint64_t, offset)
DECL_DRIVER_API_N(draw, uint32_t, vertexCount, uint32_t, instanceCount, uint32_t, firstVertex, uint32_t, firstInstance)
DECL_DRIVER_API_N(drawIndexed, uint32_t, indexCount, uint32_t, instanceCount, uint32_t, firstIndex, int32_t, vertexOffset, uint32_t, firstInstance)
DECL_DRIVER_API_N(drawIndirect, HwBuffer*, buffer, uint64_t, offset, uint32_t, drawCount, uint32_t, stride)
DECL_DRIVER_API_N(drawIndexedIndirect, HwBuffer*, buffer, uint64_t, offset, uint32_t, drawCount, uint32_t, stride)
DECL_DRIVER_API_N(dispatch, uint32_t, groupCountX,	uint32_t, groupCountY, uint32_t, groupCountZ)
DECL_DRIVER_API_N(dispatchIndirect, HwBuffer*, buffer, uint64_t, offset)

DECL_DRIVER_API_N(drawPrimitive, HwRenderPrimitive*, primitive)
DECL_DRIVER_API_N(drawBatch, RenderQueue*, renderQueue)
DECL_DRIVER_API_N(resourceBarrier, uint32_t, barrierCount, const Barrier*, pBarriers)

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#undef EXPAND

#undef PARAM_1
#undef ARG_1
#undef DECL_DRIVER_API_0
#undef DECL_DRIVER_API_R_0

#undef PARAM
#undef ARG
#undef DECL_DRIVER_API_N
#undef DECL_DRIVER_API_R_N
#undef DECL_DRIVER_API_SYNCHRONOUS_N
#undef DECL_DRIVER_API_SYNCHRONOUS_0

#undef DECL_DRIVER_API
#undef DECL_DRIVER_API_SYNCHRONOUS
#undef DECL_DRIVER_API_RETURN

#undef PAIR_ARGS_1
#undef PAIR_ARGS_2
#undef PAIR_ARGS_3
#undef PAIR_ARGS_4
#undef PAIR_ARGS_5
#undef PAIR_ARGS_6
#undef PAIR_ARGS_7
#undef PAIR_ARGS_8
#undef PAIR_ARGS_N__
#undef PAIR_ARGS_N

// No include guard!!!
