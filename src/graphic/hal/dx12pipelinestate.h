/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "graphic/hal/dx12component.h"

struct DX12PipelineStateStream
{
    CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        m_RootSignature;
    CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          m_InputLayout;
    CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    m_PrimitiveTopology;
    CD3DX12_PIPELINE_STATE_STREAM_VS                    m_VertexShader;
    CD3DX12_PIPELINE_STATE_STREAM_PS                    m_PixelShader;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  m_DSVFormat;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS m_RTVFormats;
};

class DX12PipelineState : public DX12Component<ID3D12PipelineState>
{
public:
    DX12PipelineState(
        wrl::ComPtr<ID3D12Device3> device, 
        DX12PipelineStateStream& pipelineStateStream
    );

public:
    inline wrl::ComPtr<ID3D12PipelineState> Get() { return m_PipelineState; };

private:
    wrl::ComPtr<ID3D12PipelineState> m_PipelineState;
};

