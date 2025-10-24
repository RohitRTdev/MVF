#pragma once

#include <vector>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <fstream>
#include <mutex>
#include <atomic>
#include <optional>
#include <memory>
#include <thread>
#include "math_utils.h"

struct VolumeData {
    int nx, ny, nz;
    Vector3f origin;
    Vector3f spacing;
    std::unordered_map<std::string, std::tuple<std::vector<float>, int>> scalars;
};

struct LoadProxy {
    std::ifstream file;
    VolumeData data;
    std::atomic<size_t> num_bytes_read;
    std::atomic<bool> stop_requested;
    size_t total_bytes;
    size_t total_fields_read;
    size_t total_fields;
    size_t field_size;
    std::streampos file_offset;
    bool read_failed;
    std::atomic<bool> thread_dispatched;
    std::thread worker_thread;

    bool load();
    void cancel_io();
    void complete();
};

bool readFile(const std::string& filename, std::string& out);
std::unique_ptr<LoadProxy> open_vtk_async(const std::string& filename);
