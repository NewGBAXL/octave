#if NODE_CONV_EDITOR

#pragma once

#include "Panel.h"
#include "EditorState.h"
#include "Nodes/Widgets/ModalList.h"
#include "Nodes/Widgets/PolyRect.h"

class Button;

enum class WidgetControlMode
{
    Default,
    Translate,
    Rotate,
    Scale,
    Pan
};

enum class WidgetAxisLock
{
    None,
    AxisX,
    AxisY,

    Count
};

class WidgetViewportPanel : public Panel
{
public:

    WidgetViewportPanel();
    ~WidgetViewportPanel();

    virtual void Tick(float deltaTime) override;
    virtual void HandleInput() override;

    void SetWidgetControlMode(WidgetControlMode newMode);

    void OnSelectedWidgetChanged();
    void SyncEditRootWidget();

protected:

    void HandleDefaultControls();
    void HandleTransformControls();
    void HandlePanControls();

    glm::vec2 HandleLockedCursor();
    void HandleAxisLocking();

    void SavePreTransforms();
    void RestorePreTransforms();

    Widget* FindHoveredWidget(Widget* widget, uint32_t& maxDepth, int32_t mouseX, int32_t mouseY, uint32_t depth = 0);

    float mZoom = 1.0f;
    glm::vec2 mRootOffset = { 0.0f, 0.0f };
    WidgetControlMode mControlMode = WidgetControlMode::Default;
    WidgetAxisLock mAxisLock = WidgetAxisLock::None;

    Widget* mEditRootWidget = nullptr;
    Widget* mWrapperWidget = nullptr;

    PolyRect* mSelectedRect = nullptr;
    PolyRect* mHoveredRect = nullptr;

    Text* mEditorLabel = nullptr;

    int32_t mPrevMouseX = 0;
    int32_t mPrevMouseY = 0;

    glm::vec2 mSavedOffset;
    glm::vec2 mSavedSize;
    float mSavedRotation;
};

#endif
