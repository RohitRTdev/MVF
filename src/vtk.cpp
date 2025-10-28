#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include "vtk.h"

namespace MVF {
    bool read_file(const std::string& filename, std::string& out) {
        std::ifstream file(filename, std::ios::in | std::ios::binary);
        if (!file) {
            std::cerr << "Unable to open file: " << filename << std::endl;
            return false; 
        }

        std::ostringstream ss;
        ss << file.rdbuf(); 
        out = ss.str();

        return true;
    }
    
    
#ifdef MVF_DEBUG
    VolumeData::~VolumeData() {
        std::cout << "Destroyed model object: " << filename << std::endl;
    }
#endif

    std::unique_ptr<LoadProxy> open_vtk_async(const std::string& filename) {
        auto vol = std::make_shared<VolumeData>();
        size_t total_bytes = 0;
        size_t total_fields = 0;
        std::ifstream file(filename, std::ios::binary);
        auto proxy = std::make_unique<LoadProxy>();
        if (!file) {
            std::cerr << "Failed to open " << filename << "\n";
            proxy->read_failed = true;
            return proxy;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string tag;
            if (line.rfind("DIMENSIONS", 0) == 0) {
                ss >> tag >> vol->nx >> vol->ny >> vol->nz;
            } 
            else if (line.rfind("SPACING", 0) == 0) {
                ss >> tag >> vol->spacing.x >> vol->spacing.y >> vol->spacing.z;
            } 
            else if (line.rfind("ORIGIN", 0) == 0) {
                ss >> tag >> vol->origin.x >> vol->origin.y >> vol->origin.z;
            } 
            else if (line.rfind("FIELD", 0) == 0) {
                size_t fields;
                std::string _name;
                ss >> tag >> _name >> fields;
                total_fields = fields;
                total_bytes = fields * vol->nx * vol->ny * vol->nz;
                break;
            }
        }

        vol->filename = filename;

        proxy->file_offset = file.tellg();
        proxy->file = std::move(file);
        proxy->filename = filename;
        proxy->data = vol;
        proxy->num_bytes_read = 0;
        proxy->total_bytes = total_bytes;
        proxy->total_fields_read = 0;
        proxy->total_fields = total_fields;
        proxy->field_size = (size_t)(vol->nx * vol->ny * vol->nz);
        proxy->read_failed = false;
        proxy->thread_dispatched = false;

        return proxy;
    }

    bool LoadProxy::load() {
        // We're done
        if (total_fields_read == total_fields) {
            return true;
        }

        if (thread_dispatched.load(std::memory_order_acquire)) {
            return false;
        }
        
        thread_dispatched.store(true, std::memory_order_release);
        worker_thread = std::thread([this] {
            try {
                // Read the entire file into memory at once
                file.seekg(0, std::ios::end);
                std::streamsize size = file.tellg() - file_offset;
                file.seekg(file_offset, std::ios::beg);

                std::string buffer(size, '\0');
                file.read(buffer.data(), size);
                file.close();

                const char* ptr = buffer.data();
                const char* end = ptr + buffer.size();

                std::string cur_tag;
                size_t cur_field_idx = 0;

                while (total_fields_read < total_fields && ptr < end) {
                    std::string cur_tag;
                    if (cur_field_idx == 0) {
                        // Skip empty lines
                        while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) ++ptr;

                        const char* line_start = ptr;
                        while (ptr < end && *ptr != '\n' && *ptr != '\r') ++ptr;
                        std::string line(line_start, ptr);

                        // Parse "tag comps count"
                        std::istringstream ss(line);
                        size_t comps = 0, count = 0;
                        ss >> cur_tag >> comps >> count;

                        if (count * comps != field_size) {
                            std::cerr << "Found field with tag " << cur_tag
                                    << " having different number of data points ("
                                    << count * comps << ") compared to expected field_size ("
                                    << field_size << ")" << std::endl;
                            read_failed = true;
                            return;
                        }

                        data->scalars[cur_tag] = std::make_tuple(std::vector<float>(count * comps), comps);
                    }

                    auto& dest = std::get<0>(data->scalars[cur_tag]);
                    while (ptr < end || !stop_requested.load(std::memory_order_relaxed)) {
                        // Skip whitespace
                        while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) ++ptr;
                        if (ptr >= end) break;

                        float v;
                        auto [next, ec] = std::from_chars(ptr, end, v);
                        if (ec == std::errc()) {
                            dest[cur_field_idx++] = v;
                            ptr = next;
                            num_bytes_read.fetch_add(1, std::memory_order_relaxed);
                        } else {
                            // Not a number → end of field or end of file
                            break;
                        }

                        if (cur_field_idx == field_size) {
                            total_fields_read++;
                            cur_field_idx = 0;
                            break;
                        }
                    }

                    if (stop_requested.load(std::memory_order_relaxed)) {
                        return;
                    }

                    // If we're at EOF but not done
                    if (ptr >= end && total_fields_read != total_fields) {
                        if (cur_field_idx > 0)
                            std::cerr << "Warning: incomplete field read before EOF" << std::endl;

                        read_failed = true;
                        return;
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error while loading VTK file: " << e.what() << std::endl;
                read_failed = true;
                return;
            }
        });

        return true;
    }

    void LoadProxy::cancel_io() {
        stop_requested.store(true, std::memory_order_relaxed);
    }

    void LoadProxy::complete() {
        if (!thread_dispatched.load(std::memory_order_relaxed)) {
            return;
        }
        
        if (worker_thread.joinable()) {
            worker_thread.join();
        }

        thread_dispatched.store(false, std::memory_order::release);
    }

#ifdef MVF_DEBUG
    LoadProxy::~LoadProxy() {
        std::cout << "Destroyed proxy object: " << filename << std::endl;
    }
#endif
}


