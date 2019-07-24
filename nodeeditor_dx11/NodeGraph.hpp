#ifndef NODEGRAPH_HPP
#define NODEGRAPH_HPP

#include <any>
#include <imgui.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>

namespace NodeGraph
{
    enum ConnectionType
    {
        Sequence,
        Int,
        Float,
        String,
        Vector2,
    };

    struct NodeType
    {
        std::vector< std::pair< ConnectionType, std::string > > inputs;
        std::vector< std::pair< ConnectionType, std::string > > outputs;
        bool canUserCreate = true;
    };

    struct Node
    {
        public:
            ImVec2 position;

            std::string type;

            std::vector< std::any > inputs;
            std::vector< std::any > outputs;

            bool collapsed = false;
            bool selected = false;

        private:
            friend class Graph;

            ImVec2 getInputConnectorPos( ImVec2 base, int index );

            ImVec2 getOutputConnectorPos( ImVec2 base, int index );
    };

    struct Connection
    {
        Node* other = nullptr;
        int index = 0;
        bool selected = false;
    };

    class Graph
    {
        public:
            std::vector< std::unique_ptr< Node > > nodes;
            std::unordered_map< std::string, NodeType > types;

            ImU32 gridColor = ImColor( 128, 128, 128, 32 );
            float gridSize = 64;

            void update();
            void deletePressed();

        private:
            ImVec2 scroll = ImVec2( 0, 0 );

            std::unique_ptr< Connection > connSel;
            bool connSelInput = false;

            void deselectAll();

            ImU32 getConnectorColor( ConnectionType connType );
            void doPinCircle( ImDrawList* draw, ImVec2 pos, ConnectionType connType, bool filled );
            void doPinValue( const std::string& label, ConnectionType connType, std::any& input );
    };
}

#endif // NODEGRAPH_HPP
