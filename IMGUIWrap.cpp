#include "IMGUIWrap.h"

#include <vector>

#include "d3d11.h"
#include "ShaderTypes.h"
#include "Model.h"
#include "Window.h"

CModel<ShaderTypes::VertexFormats::IMGUI> g_IMGUIMesh;
CTexture g_IMGUIFont;

void IMGUIRenderFunction(ImDrawData* draw_data)
{
    // copy all the vertex and index data into one buffer
    g_IMGUIMesh.Write(
        g_d3d.Device(),
        g_d3d.Context(),
        [&] (std::vector<ShaderTypes::VertexFormats::IMGUI>& vertexData, std::vector<unsigned long>& indexData)
        {
            if (indexData.size() < draw_data->TotalIdxCount)
                indexData.resize(draw_data->TotalIdxCount);

            if (vertexData.size() < draw_data->TotalVtxCount)
                vertexData.resize(draw_data->TotalVtxCount);

            ImDrawVert* vtx_dst = (ImDrawVert*)&vertexData[0];
            ImDrawIdx* idx_dst = (ImDrawIdx*)&indexData[0];

            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
        }
    );

    // setup shader
    const CShader& IMGUI = ShaderData::GetShader_IMGUI({});
    IMGUI.Set(g_d3d.Context());
    FillShaderParams<EShaderType::vertex>(g_d3d.Context(), IMGUI.GetVSReflector());
    FillShaderParams<EShaderType::pixel>(g_d3d.Context(), IMGUI.GetPSReflector());

    // Usually you'd setup render states but the render states it wants are the same that we use.
    // render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    size_t vtx_offset = 0;
    size_t idx_offset = 0;
    g_d3d.EnableAlphaBlend(true);
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        ImDrawList* cmd_list = draw_data->CmdLists[n];
        
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // set scissor rect
                g_d3d.SetScissor((size_t)pcmd->ClipRect.x, (size_t)pcmd->ClipRect.y, (size_t)pcmd->ClipRect.z, (size_t)pcmd->ClipRect.w);

                // bind the texture
                CTexture* texture = (CTexture*)pcmd->TextureId;
                ID3D11ShaderResourceView* srv = texture->GetSRV();
                g_d3d.Context()->PSSetShaderResources(0, 1, &srv);

                // draw the mesh
                g_IMGUIMesh.Set(g_d3d.Context());
                g_d3d.DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // reset scissor and unbind the texture
    g_d3d.ClearScissor();
    ID3D11ShaderResourceView* srv = nullptr;
    g_d3d.Context()->PSSetShaderResources(0, 1, &srv);
    g_d3d.EnableAlphaBlend(false);
}

bool InitIMGUI()
{
    bool writeOK = g_IMGUIMesh.Create(
        g_d3d.Device(),
        [] (std::vector<ShaderTypes::VertexFormats::IMGUI>& vertexData, std::vector<unsigned long>& indexData)
        {
            vertexData.resize(1);
            indexData.resize(1);

            for (size_t i = 0; i < 1; ++i)
                indexData[i] = (unsigned long) i;
        }
    );

    if (!writeOK)
    {
        ReportError("Could not create imgui mesh\n");
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = c_width;
    io.DisplaySize.y = c_height;
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    io.RenderDrawListsFn = IMGUIRenderFunction;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.ImeWindowHandle = WindowGetHWND();

    // Load texture atlas (there is a default font so you don't need to care about choosing a font yet)
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    if (!g_IMGUIFont.LoadFromPixels(g_d3d.Device(), g_d3d.Context(), pixels, width, height))
        return false;

    // Store texture pointer/identifier (whatever your engine uses) in 'io.Fonts->TexID'. This will be passed back to your via the renderer.
    io.Fonts->TexID = (void*)&g_IMGUIFont;
    return true;
}