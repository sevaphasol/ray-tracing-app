#pragma once

#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"
#include "zemax/model/rendering/vector3.hpp"
#include <cstdint>
#include <sys/types.h>

namespace zemax {

struct Config
{
    struct Common
    {
        struct Font
        {
            static inline const char* const Name  = "assets/JetBrainsMono-Regular.ttf";
            static inline const dr4::Color  Color = { 255, 255, 255, 255 };
        };
    };

    struct Window
    {
        static inline const size_t      Width           = 2150.0f;
        static inline const size_t      Height          = 900.0f;
        static inline const char* const Title           = "Zemax";
        static inline const dr4::Color  BackgroundColor = { 0, 0, 0, 255 };
    };

    struct CameraPanel
    {
        static inline const dr4::Vec2f Position        = { 50.0f, 50.0f };
        static inline const dr4::Vec2f Size            = { 380.0f, 385.0f };
        static inline const dr4::Color BackgroundColor = { 18, 18, 18, 255 };
        static inline const dr4::Color BorderColor     = { 118, 185, 0, 255 };
        static inline const float      BorderThickness = 2.0f;

        struct Button
        {
            static inline const dr4::Vec2f Size = { 100.0f, 50.0f };

            static inline const dr4::Color DefaultColor = { 30, 30, 30, 255 };
            static inline const dr4::Color HoveredColor = { 50, 70, 30, 255 };
            static inline const dr4::Color PressedColor = { 100, 150, 0, 255 };

            static inline const char* const FontName  = "assets/JetBrainsMono-Regular.ttf";
            static inline const dr4::Color  FontColor = { 220, 220, 220, 255 };
            static inline const size_t      FontSize  = 15;

            struct MvL
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 80.0f };
                static inline const char* const Title    = "left";
            };

            struct MvR
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 140.0f };
                static inline const char* const Title    = "right";
            };

            struct MvU
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 80.0f };
                static inline const char* const Title    = "up";
            };

            struct MvD
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 140.0f };
                static inline const char* const Title    = "down";
            };

            struct MvF
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 80.0f };
                static inline const char* const Title    = "fwd";
            };

            struct MvB
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 140.0f };
                static inline const char* const Title    = "bwd";
            };

            struct RtL
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 220.0f };
                static inline const char* const Title    = "rot left";
            };

            struct RtR
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 280.0f };
                static inline const char* const Title    = "rot right";
            };

            struct RtU
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 220.0f };
                static inline const char* const Title    = "rot up";
            };

            struct RtD
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 280.0f };
                static inline const char* const Title    = "rot down";
            };

            struct ScaleUp
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 220.0f };
                static inline const char* const Title    = "scale up";
            };

            struct ScaleDown
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 280.0f };
                static inline const char* const Title    = "scale down";
            };
        };
    };

    struct ControlPanel
    {
        static inline const dr4::Vec2f Position        = { 50.0f, 465.0f };
        static inline const dr4::Vec2f Size            = { 380.0f, 385.0f };
        static inline const dr4::Color BackgroundColor = { 18, 18, 18, 255 };
        static inline const dr4::Color BorderColor     = { 118, 185, 0, 255 };
        static inline const float      BorderThickness = 2.0f;

        struct Button
        {
            static inline const dr4::Vec2f Size = { 100.0f, 50.0f };

            static inline const dr4::Color DefaultColor = { 30, 30, 30, 255 };
            static inline const dr4::Color HoveredColor = { 50, 70, 30, 255 };
            static inline const dr4::Color PressedColor = { 100, 150, 0, 255 };

            static inline const char* const FontName  = "assets/JetBrainsMono-Regular.ttf";
            static inline const dr4::Color  FontColor = { 220, 220, 220, 255 };
            static inline const size_t      FontSize  = 15;

            struct RotateObjLeft
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 80.0f };
                static inline const char* const Title    = "Y-";
            };

            struct RotateObjRight
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 140.0f };
                static inline const char* const Title    = "Y+";
            };

            struct RotateObjUp
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 80.0f };
                static inline const char* const Title    = "X+";
            };

            struct RotateObjDown
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 140.0f };
                static inline const char* const Title    = "X-";
            };

            struct RotateObjRollLeft
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 80.0f };
                static inline const char* const Title    = "Z-";
            };

            struct RotateObjRollRight
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 140.0f };
                static inline const char* const Title    = "Z+";
            };

            struct MoveObjLeft
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 220.0f };
                static inline const char* const Title    = "left";
            };

            struct MoveObjRight
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 280.0f };
                static inline const char* const Title    = "right";
            };

            struct MoveObjUp
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 220.0f };
                static inline const char* const Title    = "up";
            };

            struct MoveObjDown
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 280.0f };
                static inline const char* const Title    = "down";
            };

            struct MoveObjForward
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 220.0f };
                static inline const char* const Title    = "fwd";
            };

            struct MoveObjBackward
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 280.0f };
                static inline const char* const Title    = "bwd";
            };

            struct AddObj
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 300.0f };
                static inline const char* const Title    = "Add obj";
            };

            struct CopyObj
            {
                static inline const dr4::Vec2f  Position = { 140.0f, 300.0f };
                static inline const char* const Title    = "Copy obj";
            };

            struct EditObj
            {
                static inline const dr4::Vec2f  Position = { 250.0f, 300.0f };
                static inline const char* const Title    = "Edit obj";
            };

            struct DeleteObj
            {
                static inline const dr4::Vec2f  Position = { 30.0f, 360.0f };
                static inline const char* const Title    = "Delete obj";
            };
        };
    };

    struct Scene
    {
        static inline const dr4::Vec2f Position        = { 475.0f, 50.0f };
        static inline const dr4::Vec2f Size            = { 1200.0f, 800.0f };
        static inline const dr4::Color BackgroundColor = { 10, 10, 10, 255 };

        struct ObjInfoPanel
        {
            static inline const dr4::Vec2f Size             = { 215.0f, 65.0f };
            static inline const dr4::Color FontColor        = { 255, 255, 255, 255 };
            static inline const uint32_t   FontSize         = 12;
            static inline const dr4::Color FillColor        = { 32, 32, 32, 255 };
            static inline const dr4::Color OutlineColor     = { 64, 64, 64, 255 };
            static inline const float      OutlineThickness = 2.0f;
        };
    };

    struct Camera
    {
        static inline const model::Vector3f Position = { 0.0f, 0.0f, 0.0f };

        static inline const float MoveFactor    = 0.1f;
        static inline const float ObjMoveFactor = 0.1f;
        static inline const float RotateFactor  = 0.03f;
        static inline const float ScaleFactor   = 0.03f;
    };
};

} // namespace zemax
