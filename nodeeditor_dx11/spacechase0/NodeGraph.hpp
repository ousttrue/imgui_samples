#ifndef NODEGRAPH_HPP
#define NODEGRAPH_HPP

#include <any>
#include <imgui.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include "Node.hpp"

namespace spacechase0
{
    class Graph
    {
        public:
            Graph();
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
