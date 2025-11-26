#pragma once

#include <variant>
#include <cstddef>
#include <string>
#include "shapes.h"

constexpr float AXIS_LENGTH = 1.6f;
constexpr float AXIS_WIDTH = 0.015f;
constexpr size_t MAX_COLORS = 4;

namespace MVF {
    enum class TraitType {
        POINT,
        RANGE
    };

    struct Interval {
        float left, right;
        IntervalSelector mesh;
    };

    struct Polygon {
        float x_top, y_top;
        float width, height;
        PolySelector mesh;
    };

    enum class RangeType {
        INTERVAL,
        POLYGON
    };

    struct Range {
        RangeType type;
        std::variant<Interval, Polygon> range;
    };
    
    struct Trait {
        TraitType type;
        std::variant<Point, Range> data;
        size_t color_id;
    };

    struct AxisDesc {
        std::string comp_name;
        std::string display_name;
        float (*derive)(float comp);
    };

    struct AxisDescMeta {
        AxisDesc desc;
        float min_val, max_val;
    };

    extern const Vector3f global_color_pallete[MAX_COLORS];
}