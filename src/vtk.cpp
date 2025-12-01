#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include "vtk.h"
#include "error.h"

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
    
    VolumeData::~VolumeData() {
#ifdef MVF_DEBUG
        std::cout << "Destroyed model object: " << filename << std::endl;
#endif
    }

    std::unique_ptr<LoadProxy> open_vtk_async(const std::string& filename) {
        auto vol = std::make_shared<VolumeData>();
        size_t total_bytes = 0;
        size_t total_fields = 0;
        std::ifstream file(filename, std::ios::binary);
        auto proxy = std::make_unique<LoadProxy>();
        proxy->read_failed = true;
        if (!file) {
            std::cerr << "Failed to open " << filename << "\n";
            return proxy;
        }

        std::array<std::string, 4> header = {"# vtk DataFile Version 5.1", "vtk output", "ASCII", 
                    "DATASET STRUCTURED_POINTS"};
        
        size_t header_idx = 0;
        std::string line, tag;
        auto clean_line = [&](std::string& s) {
            if (!s.empty() && s.back() == '\r')
                s.pop_back();
        };

        while (header_idx < header.size()) {
            if (!std::getline(file, line)) {
    #ifdef MVF_DEBUG
                std::cerr << "Missing line: " << header[header_idx] << std::endl;
    #endif
                return proxy;
            }

            clean_line(line);
            if (line != header[header_idx]) {
    #ifdef MVF_DEBUG
                std::cerr << "Expected line: " << header[header_idx] << std::endl;
    #endif
                return proxy;
            }

            header_idx++;
        }

        if (!std::getline(file, line)) {
#ifdef MVF_DEBUG
            std::cerr << "Missing DIMENSIONS line" << std::endl;
#endif
            return proxy;
        }
        {
            std::istringstream ss(line);
            if (!(ss >> tag >> vol->nx >> vol->ny >> vol->nz) || tag != "DIMENSIONS") {
#ifdef MVF_DEBUG
                std::cerr <<"Invalid DIMENSIONS format" << std::endl;
#endif
                return proxy;
            }
        }

        if (!std::getline(file, line)) {
#ifdef MVF_DEBUG
            std::cerr << "Missing SPACING line" << std::endl;
#endif
            return proxy;
        }
        {
            std::istringstream ss(line);
            if (!(ss >> tag >> vol->spacing.x >> vol->spacing.y >> vol->spacing.z) || tag != "SPACING") {
#ifdef MVF_DEBUG    
                std::cerr << "Invalid SPACING format" << std::endl;
#endif
                return proxy;
            }
        }

        if (!std::getline(file, line)) {
#ifdef MVF_DEBUG
            std::cerr << "Missing ORIGIN line" << std::endl;
#endif
            return proxy;
        } 
        {
            std::istringstream ss(line);
            if (!(ss >> tag >> vol->origin.x >> vol->origin.y >> vol->origin.z) || tag != "ORIGIN") {
#ifdef MVF_DEBUG
                std::cerr << "Invalid ORIGIN format" << std::endl;
#endif
                return proxy;
            }
        }
        
        if (!std::getline(file, line)) {
#ifdef MVF_DEBUG
            std::cerr << "Missing POINT_DATA line" << std::endl;
#endif
            return proxy;
        }

        if (!std::getline(file, line)) {
#ifdef MVF_DEBUG
            std::cerr << "Missing FIELD line" << std::endl;
#endif
            return proxy;
        }
        {
            std::istringstream ss(line);
            std::string _name;
            size_t fields = 0;

            if (!(ss >> tag >> _name >> fields) || tag != "FIELD") {
#ifdef MVF_DEBUG
                std::cerr << "Invalid FIELD format" << std::endl;
#endif
                return proxy;
            }

            total_fields = fields;
            total_bytes = fields * vol->nx * vol->ny * vol->nz;
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
        proxy->stop_requested = false;

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

                size_t cur_field_idx = 0;
                std::string cur_tag;

                while (total_fields_read < total_fields && ptr < end) {
                    if (cur_field_idx == 0) {
                        // Skip whitespace/newlines
                        while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) ++ptr;
                        // Read header line
                        const char* line_start = ptr;
                        while (ptr < end && *ptr != '\n' && *ptr != '\r') ++ptr;
                        std::string line(line_start, ptr);

                        std::istringstream ss(line);
                        size_t comps = 0, count = 0;
                        ss >> cur_tag >> comps >> count;

                        if (comps != 1) {
                            std::cerr << "Field " << cur_tag << " has more than one component. This is not supported right now..." << std::endl;
                            read_failed = true;
                            return;
                        }

                        if (count * comps != field_size) {
                            std::cerr << "Field " << cur_tag << " size mismatch (" << count*comps << " vs " << field_size << ")" << std::endl;
                            read_failed = true;
                            return;
                        }

                        data->scalars[cur_tag] = std::vector<float>(count * comps);
                        ++ptr; // advance past newline if present
                        cur_field_idx = 0;
                    }

                    auto& dest = data->scalars[cur_tag];

                    while (ptr < end && cur_field_idx < field_size) {
                        // Skip whitespace
                        while (ptr < end && std::isspace(static_cast<unsigned char>(*ptr))) ++ptr;
                        if (ptr >= end) break;
                        float v;
                        auto [next, ec] = std::from_chars(ptr, end, v);
                        if (ec != std::errc()) break;
                        dest[cur_field_idx++] = v;
                        ptr = next;
                        num_bytes_read.fetch_add(1, std::memory_order_relaxed);
                    }

                    if (cur_field_idx == field_size) {
                        total_fields_read++;
                        cur_field_idx = 0;
                    }

                    if (stop_requested.load(std::memory_order_relaxed)) {
                        return;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error while loading VTK file: " << e.what() << std::endl;
                read_failed = true;
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

        thread_dispatched.store(false, std::memory_order_release);
    }

    void LoadProxy::reset() {
        if (worker_thread.joinable()) worker_thread.join();
        thread_dispatched.store(false, std::memory_order_relaxed);
        num_bytes_read.store(0, std::memory_order_relaxed);
        total_fields_read = 0;
        read_failed = false;
        stop_requested.store(false, std::memory_order_relaxed);
    }

    LoadProxy::~LoadProxy() {
#ifdef MVF_DEBUG
        std::cout << "Destroyed proxy object: " << filename << std::endl;
#endif
        if (worker_thread.joinable()) worker_thread.join();
    }
}


