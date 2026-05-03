#ifndef STREAM_REGISTRY_H
#define STREAM_REGISTRY_H

#include "camera_types.hpp"
#include "spdlog/spdlog.h"
#include <iostream>
#include <memory>
#include <map>
#include <mutex>
#include <peel/Gst/Gst.h>
#include <tl/expected.hpp>


class StreamRegistry
{
  public:
    StreamRegistry();
    ~StreamRegistry();

    tl::expected<std::string, equinox::camera::CamErrorDetails> create_stream(const std::string, const char *, peel::RefPtr<peel::Gst::Device>);
    tl::expected<void, equinox::camera::CamErrorDetails> remove_stream(const std::string);
  private:
    std::mutex stream_mutex_;
    std::map<std::string, std::shared_ptr<equinox::camera::StreamInstance>> stream_map_;
};

#endif