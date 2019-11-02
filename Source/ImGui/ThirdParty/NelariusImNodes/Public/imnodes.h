#pragma once

#include <stddef.h>

struct ImVec2;

namespace imnodes
{
enum ColorStyle
{
    ColorStyle_NodeBackground = 0,
    ColorStyle_NodeBackgroundHovered,
    ColorStyle_NodeBackgroundSelected,
    ColorStyle_NodeOutline,
    ColorStyle_TitleBar,
    ColorStyle_TitleBarHovered,
    ColorStyle_TitleBarSelected,
    ColorStyle_Link,
    ColorStyle_LinkHovered,
    ColorStyle_LinkSelected,
    ColorStyle_Pin,
    ColorStyle_PinHovered,
    ColorStyle_PinOutline,
    ColorStyle_BoxSelector,
    ColorStyle_BoxSelectorOutline,
    ColorStyle_GridBackground,
    ColorStyle_GridLine,
    ColorStyle_Count
};

// Flags for controlling how the nodes are rendered.
enum Flags
{
    Flags_None = 0,
    Flags_NodeOutline = 1 << 0,
    Flags_PinOutline = 1 << 1
};

struct Style
{
    // by default, set to Flags_NodeOutline | Flags_PinOutline
    Flags flags;
    // Set these mid-frame using Push/PopColorStyle. You can index this color
    // array with with a ColorStyle enum value.
    unsigned int colors[ColorStyle_Count];
};

// An editor context corresponds to a set of nodes in a single workspace
// (created with a single Begin/EndNodeEditor pair)
//
// By default, the library creates an editor context behind the scenes, so
// using any of the imnodes functions doesn't require you to explicitly create a
// context.
struct EditorContext;

IMGUI_API EditorContext* ditorContextCreate();
IMGUI_API void EditorContextFree(EditorContext*);
IMGUI_API void EditorContextSet(EditorContext*);

// Initialize the node editor system.
IMGUI_API void Initialize();
IMGUI_API void Shutdown();

// Returns the global style struct. See the struct declaration for default
// values.
IMGUI_API Style& GetStyle();
// Style presets matching the dear imgui styles of the same name.
IMGUI_API void StyleColorsDark(); // on by default
IMGUI_API void StyleColorsClassic();
IMGUI_API void StyleColorsLight();

// The top-level function call. Call this before calling BeginNode/EndNode.
// Calling this function will result the node editor grid workspace being
// rendered.
IMGUI_API void BeginNodeEditor();
IMGUI_API void EndNodeEditor();

// Use PushColorStyle and PopColorStyle to modify Style::colors mid-frame.
IMGUI_API void PushColorStyle(ColorStyle item, unsigned int color);
IMGUI_API void PopColorStyle();

IMGUI_API void BeginNode(int id);
IMGUI_API void EndNode();

// Attributes are ImGui UI elements embedded within the node. Attributes have
// circular pins rendered next to them. Links are created between pins.
//
// Input and output attributes are otherwise the same, except that pins are
// rendered on the left of the node for input attributes, and on the right side
// for output attributes.
//
// The attribute ids must be unique.
IMGUI_API void BeginInputAttribute(int id);
IMGUI_API void BeginOutputAttribute(int id);
IMGUI_API void EndAttribute();

// Render a link between attributes.
// The attributes ids used here must match the ids used in
// Begin(Input|Output)Attribute function calls. The order of start_attr and
// end_attr doesn't make a difference for rendering the link.
IMGUI_API void Link(int id, int start_attr, int end_attr);

// Set's the node's position corresponding to the node id. You can even set the
// position before the node has been created with BeginNode().
IMGUI_API void SetNodePos(int node_id, const ImVec2& pos);
// Set the node name corresponding to the node id. The node name is displayed in
// the node's title bar.
IMGUI_API void SetNodeName(int node_id, const char* name);

// The following functions return true if a UI element is being hovered over by
// the mouse cursor. Assigns the id of the UI element being hovered over to the
// function argument. Use these functions after EndNodeEditor() has been called.
IMGUI_API bool IsNodeHovered(int* node_id);
IMGUI_API bool IsLinkHovered(int* link_id);
IMGUI_API bool IsPinHovered(int* attribute_id);

// Use The following two functions to query the number of selected nodes or
// links in the current editor. Use after calling EndNodeEditor().
IMGUI_API int NumSelectedNodes();
IMGUI_API int NumSelectedLinks();
// Get the selected node/link ids. The pointer argument should point to an
// integer array with at least as many elements as the respective
// NumSelectedNodes/NumSelectedLinks function call returned.
IMGUI_API void GetSelectedNodes(int* node_ids);
IMGUI_API void GetSelectedLinks(int* link_ids);

// Was the previous attribute active? This will continuously return true while
// the left mouse button is being pressed over the UI content of the attribute.
IMGUI_API bool IsAttributeActive();
// Was any attribute active? If so, sets the active attribute id to the output
// function argument.
IMGUI_API bool IsAnyAttributeActive(int* attribute_id = 0);

// The following functions should be used after calling EndNodeEditor().
//
// Is the user dragging a new link?
IMGUI_API bool IsLinkStarted(int* started_at_attr);
// Did the user drop the new link before connecting it to a second attribute?
IMGUI_API bool IsLinkDropped();
// Did the user create a new link?
IMGUI_API bool IsLinkCreated(int* started_at_attr, int* ended_at_attr);

// Use the following functions to write the editor context's state to a string,
// or directly to a file. The editor context is serialized in the INI file
// format.

IMGUI_API const char* SaveCurrentEditorStateToMemory(size_t* data_size = NULL);
IMGUI_API const char* SaveEditorStateToMemory(
    const EditorContext* editor,
    size_t* data_size = NULL);

IMGUI_API void LoadCurrentEditorStateFromMemory(const char* data, size_t data_size);
IMGUI_API void LoadEditorStateFromMemory(
    EditorContext* editor,
    const char* data,
    size_t data_size);

IMGUI_API void SaveCurrentEditorStateToDisk(const char* file_name);
IMGUI_API void SaveEditorStateToDisk(const EditorContext* editor, const char* file_name);

IMGUI_API void LoadCurrentEditorStateFromDisk(const char* file_name);
IMGUI_API void LoadEditorStateFromDisk(EditorContext* editor, const char* file_name);

} // namespace imnodes
