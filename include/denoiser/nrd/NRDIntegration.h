/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

// IMPORTANT: these files must be included beforehand:
//    NRD.h
//    NRIDescs.h
//    Extensions/NRIHelper.h
//    Extensions/NRIWrapperD3D11.h
//    Extensions/NRIWrapperD3D12.h
//    Extensions/NRIWrapperVK.h

#include <array>
#include <vector>
#include <map>

#define NRD_INTEGRATION_MAJOR 1
#define NRD_INTEGRATION_MINOR 6
#define NRD_INTEGRATION_DATE "30 April 2023"
#define NRD_INTEGRATION 1

#define NRD_INTEGRATION_DEBUG_LOGGING 0

#ifndef NRD_INTEGRATION_ASSERT
    #include <assert.h>
    #define NRD_INTEGRATION_ASSERT(expr, msg) (assert(expr && msg))
#endif

// User inputs / outputs are not mipmapped, thus only 1 entry is needed.
// "TextureTransitionBarrierDesc::texture" is used to store the resource.
// "TextureTransitionBarrierDesc::next..." are used to represent the current state of the subresource.
struct NrdIntegrationTexture
{
    nri::TextureTransitionBarrierDesc* subresourceStates;
    nri::Format format;
};

typedef std::array<NrdIntegrationTexture, (size_t)nrd::ResourceType::MAX_NUM - 2> NrdUserPool;

// User pool must contain valid entries only for resources, which are required for requested denoisers, but
// the entire pool must be zero-ed during initialization
inline void NrdIntegration_SetResource(NrdUserPool& pool, nrd::ResourceType slot, const NrdIntegrationTexture& texture)
{
    NRD_INTEGRATION_ASSERT( texture.subresourceStates->texture != nullptr, "Invalid texture!" );
    NRD_INTEGRATION_ASSERT( texture.format != nri::Format::UNKNOWN, "Invalid format!" );

    pool[(size_t)slot] = texture;
}

class NrdIntegration
{
public:
    // The application must provide number of buffered frames, it's needed to guarantee that
    // constant data and descriptor sets are not overwritten while being executed on the GPU.
    // Usually it's 2-3 frames.
    NrdIntegration(uint32_t bufferedFramesNum, const char* persistentName = "") :
        m_Name(persistentName)
        , m_BufferedFramesNum(bufferedFramesNum)
    {}

    ~NrdIntegration()
    { NRD_INTEGRATION_ASSERT( m_NRI == nullptr, "m_NRI must be NULL at this point!" ); }

    // There is no "Resize" functionality, because NRD full recreation costs nothing.
    // The main cost comes from render targets resizing, which needs to be done in any case
    // (call Destroy beforehand)
    bool Initialize(const nrd::InstanceCreationDesc& instanceCreationDesc, nri::Device& nriDevice,
        const nri::CoreInterface& nriCore, const nri::HelperInterface& nriHelper);

    // Must be called once on a frame start
    void NewFrame(uint32_t frameIndex);

    // Explcitly calls eponymous NRD API functions
    bool SetCommonSettings(const nrd::CommonSettings& commonSettings);
    bool SetDenoiserSettings(nrd::Identifier denoiser, const void* denoiserSettings);

    // Better use "enableDescriptorCaching = true" if resources are not changing between frames
    // (i.e. not suballocated from a heap)
    void Denoise(const nrd::Identifier* denoisers, uint32_t denoisersNum,
        nri::CommandBuffer& commandBuffer, const NrdUserPool& userPool,
        bool enableDescriptorCaching
    );

    // This function assumes that the device is in the IDLE state, i.e. there is no work in flight
    void Destroy();

    // Should not be called explicitly, unless you want to reload pipelines
    void CreatePipelines();

    // Helpers
    inline double GetTotalMemoryUsageInMb() const
    { return double(m_PermanentPoolSize + m_TransientPoolSize) / (1024.0 * 1024.0); }

    inline double GetPersistentMemoryUsageInMb() const
    { return double(m_PermanentPoolSize) / (1024.0 * 1024.0); }

    inline double GetAliasableMemoryUsageInMb() const
    { return double(m_TransientPoolSize) / (1024.0 * 1024.0); }

private:
    NrdIntegration(const NrdIntegration&) = delete;

    void CreateResources();
    void AllocateAndBindMemory();
    void Dispatch
    (
        nri::CommandBuffer& commandBuffer, nri::DescriptorPool& descriptorPool,
        const nrd::DispatchDesc& dispatchDesc, const NrdUserPool& userPool,
        bool enableDescriptorCaching
    );

private:
    std::vector<NrdIntegrationTexture> m_TexturePool;
    std::map<uint64_t, nri::Descriptor*> m_Descriptors;
    std::vector<nri::TextureTransitionBarrierDesc> m_ResourceState;
    std::vector<nri::PipelineLayout*> m_PipelineLayouts;
    std::vector<nri::Pipeline*> m_Pipelines;
    std::vector<nri::Memory*> m_MemoryAllocations;
    std::vector<nri::Descriptor*> m_Samplers;
    std::array<nri::DescriptorPool*, 16> m_DescriptorPools = {};
    const nri::CoreInterface* m_NRI = nullptr;
    const nri::HelperInterface* m_NRIHelper = nullptr;
    nri::Device* m_Device = nullptr;
    nri::Buffer* m_ConstantBuffer = nullptr;
    nri::Descriptor* m_ConstantBufferView = nullptr;
    nrd::Instance* m_Instance = nullptr;
    const char* m_Name = nullptr;
    uint64_t m_PermanentPoolSize = 0;
    uint64_t m_TransientPoolSize = 0;
    uint64_t m_ConstantBufferSize = 0;
    uint32_t m_ConstantBufferViewSize = 0;
    uint32_t m_ConstantBufferOffset = 0;
    uint32_t m_BufferedFramesNum = 0;
    uint32_t m_DescriptorPoolIndex = 0;
    bool m_IsShadersReloadRequested = false;
};

#define NRD_INTEGRATION_ABORT_ON_FAILURE(result) if ((result) != nri::Result::SUCCESS) NRD_INTEGRATION_ASSERT(false, "Abort on failure!")
