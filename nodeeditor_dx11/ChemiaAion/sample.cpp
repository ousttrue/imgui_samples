#include "sample.h"
#include "Nodes.h"

namespace ChemiaAion
{
    void NodeEditor()
    {
        static ImGui::Nodes nodes_;
        ImGui::Begin("Nodes");
        nodes_.ProcessNodes();
        ImGui::End();
    }
}