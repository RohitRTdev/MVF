#include <ranges>
#include "attrib.h"
#include "renderer.h"

namespace MVF {
    void AttribRenderer::generate_freq_distribution() {
        if (descriptors.size() != 1) {
            throw std::runtime_error("generate_freq_distribution() called with descriptors.size() != 1");
        }

        constexpr size_t samples = 20, sample_period = 1;
        constexpr float scale_factor = 0.5f;
        auto& field1 = data->scalars[descriptors[0].desc.comp_name];  
    
        auto get_bin_idx = [samples, this] (float val) {
            return std::min(samples - 1, 
                static_cast<size_t>((val - descriptors[0].min_val) / ((descriptors[0].max_val - descriptors[0].min_val) / samples)));  
        };

        // We collect the freq of each data point into one of 'sample' bins
        std::vector<size_t> bins;
        size_t max_val = 0;
        bins.resize(samples);

        for (size_t x_idx = 0; x_idx < field1.size(); x_idx += sample_period) {
            auto bin_idx = get_bin_idx(field1[x_idx]);  
            bins[bin_idx]++;
        
            max_val = std::max(bins[bin_idx], max_val);
        }

        // Generate the mesh
        auto bin_width = AXIS_LENGTH / samples;
        constexpr auto start_point = -AXIS_LENGTH / 2;
        dist_plot_lines.clear();
        dist_plot_solid.clear();
        for (size_t bin = 0; bin < samples; bin++) {
            auto bin_height = (static_cast<float>(bins[bin]) / max_val) * scale_factor;
            add_rect(dist_plot_solid, start_point + bin * bin_width, AXIS_WIDTH / 2, bin_width, bin_height);
            add_rect_outline(dist_plot_lines, start_point + bin * bin_width, AXIS_WIDTH / 2, bin_width, bin_height);
        }
    }

    void AttribRenderer::generate_scatter_plot() {
        if (descriptors.size() != 2) {
            throw std::runtime_error("generate_scatter_plot() called with descriptors.size() != 2");
        }

        constexpr size_t sample_period = 100;

        auto& field1 = data->scalars[descriptors[0].desc.comp_name];  
        auto& field2 = data->scalars[descriptors[1].desc.comp_name];  

        const auto num_points = field1.size() / sample_period; 
#ifdef MVF_DEBUG
        std::cout << "Total vertices for 2d scatter plot: " << num_points << std::endl;
#endif
        scatter_plot.clear();
        scatter_plot.reserve(num_points);
        for (size_t pt_idx = 0; pt_idx < field1.size(); pt_idx += sample_period) {
            // Convert the attribute space point to our screen space
            auto x_norm = 2 * (field1[pt_idx] - descriptors[0].min_val) / (descriptors[0].max_val - descriptors[0].min_val) - 1;
            auto y_norm = 2 * (field2[pt_idx] - descriptors[1].min_val) / (descriptors[1].max_val - descriptors[1].min_val) - 1;
            if (std::abs(x_norm) >= AXIS_LENGTH / 2 || std::abs(y_norm) >= AXIS_LENGTH / 2) {
                continue;
            }
        
            scatter_plot.push_back(Vector2f(x_norm, y_norm));
        }
    }

}    