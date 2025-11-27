#pragma once

#include <variant>
#include <cstddef>
#include <string>
#include <vector>
#include "shapes.h"

constexpr float AXIS_LENGTH = 1.6f;
constexpr float AXIS_WIDTH = 0.015f;

namespace MVF {
    enum class TraitType {
        POINT,
        RANGE,
        PARALLEL_POINT // N-D point selected across parallel axes
    };

#pragma pack(push, 1)
    struct Point {
        float x, y;
    };
#pragma pack(pop)

    // N-D point for parallel coordinates selection (stores world-space Y per axis initially)
    struct NDPoint {
        std::vector<float> ys; // size equals number of axes
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

    // Hyper-rectangular selection across N parallel axes (stores world-space Y ranges per axis initially)
    struct HyperBox {
        std::vector<std::pair<float, float>> yranges; // per-axis [ymin, ymax] in world coords initially
    };

    enum class RangeType {
        INTERVAL,
        POLYGON,
        HYPERBOX // N-D axis-aligned box across parallel coordinates
    };

    struct Range {
        RangeType type;
        std::variant<Interval, Polygon, HyperBox> range;
    };
    
    struct Trait {
        TraitType type;
        std::variant<Point, Range, NDPoint> data;
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