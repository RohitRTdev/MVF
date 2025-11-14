#pragma once

#include <variant>
#include <cstddef>
#include <string>

constexpr float AXIS_LENGTH = 1.6f;

namespace MVF {
    enum class TraitType {
        POINT,
        RANGE
    };

#pragma pack(push, 1)
    struct Point {
        float x, y;
    };
#pragma pack(pop)

    struct Interval {
        float left, right;
        IntervalSelector mesh;
    };

    struct Polygon {
        float x_left, y_top;
        float x_right, y_bottom;
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
}