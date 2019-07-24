namespace ChemiaAion
{

enum NodesState : uint32_t
{
    NodesState_Default = 0,
    NodesState_Block, // helper: just block all states till next update (frame)
    NodesState_HoverIO,
    NodesState_HoverConnection,
    NodesState_HoverNode,
    NodesState_DragingInput,
    NodesState_DragingInputValid,
    NodesState_DragingOutput,
    NodesState_DragingOutputValid,
    NodesState_DragingConnection,
    NodesState_DragingSelected,
    NodesState_SelectingEmpty,
    NodesState_SelectingValid,
    NodesState_SelectingMore,
    NodesState_Selected,
    NodesState_SelectedConnection
};

struct NodesElement
{
    NodesState state_;

    ImVec2 position_;
    ImRect rect_;

    Node *node_;
    Connection *connection_;

    NodesElement()
    {
        Reset();
    }

    void Reset(NodesState state = NodesState_Default)
    {
        state_ = state;

        position_ = ImVec2(0.0f, 0.0f);
        rect_ = ImRect(0.0f, 0.0f, 0.0f, 0.0f);

        node_ = nullptr;
        connection_ = nullptr;
    }
};

} // namespace ChemiaAion