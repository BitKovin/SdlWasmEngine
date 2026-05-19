#pragma once

class EditorExternalData
{

public:

    enum class EditMode : int
    {
        None = 0,
        Select = 1,
        BrushDraw = 2
    };
    static inline EditMode editMode = EditMode::Select;

    static inline bool SnapToGrid = true;
    static inline float GridSpacing = 1.0f;

    static inline bool SubtractiveBrush = false;

};
