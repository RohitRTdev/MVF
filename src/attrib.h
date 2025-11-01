#pragma once

#include <variant>
#include <cstddef>
#include <string>

namespace MVF {
    enum class TraitType {
        POINT,
        RANGE
    };

    struct Point {
        float x, y;
    };

    struct Interval {
        float left, right;
    };

    struct Polygon {
        float x_left, y_top;
        float x_right, y_bottom;
    };

    struct Range {
        std::variant<Interval, Polygon> range;
    };
    
    struct Trait {
        TraitType type;
        size_t num_dimensions;
        std::variant<Point, Range> data;
    };

    struct AxisDesc {
        std::string_view comp_name;
        std::string display_name;
        float (*derive)(float comp);
    };
}