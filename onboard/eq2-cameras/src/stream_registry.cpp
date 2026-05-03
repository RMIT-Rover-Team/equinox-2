#include "stream_registry.hpp"

using namespace equinox::camera;
using namespace peel;

StreamRegistry::StreamRegistry()
{

}
StreamRegistry::~StreamRegistry()
{

}

tl::expected<std::string, equinox::camera::CamErrorDetails> StreamRegistry::create_stream(const std::string uid, const char * name, RefPtr<Gst::Device> device)
{
  auto pipeline = Gst::Pipeline::create(name);
  if (!pipeline) return MAKE_CAM_ERROR_EXPECTED(CamError::PipelineCreationFailed, "Pipeline was unable to be created");
  
  auto float_ptr_src = device->create_element("source");
  if (!float_ptr_src) return MAKE_CAM_ERROR_EXPECTED(CamError::SourceCreationFailed, "Pipeline source was unable to be created");

  peel::RefPtr<peel::Gst::Element> ref_ptr_src = float_ptr_src;  // upcast to ref ptr for pipeline
  pipeline->add(ref_ptr_src); 

  auto stream = std::make_shared<StreamInstance>();

  stream->pipeline = pipeline;
  stream->source = ref_ptr_src;

  auto source = pipeline->get_by_name("source");
  if (source != ref_ptr_src) return MAKE_CAM_ERROR_EXPECTED(CamError::AdditionFailed, "Failed to add source element to pipeline");

  {
    std::lock_guard<std::mutex> lock(stream_mutex_);
    stream_map_.insert_or_assign(uid, std::move(stream));
  }
  return uid;
}

tl::expected<void, equinox::camera::CamErrorDetails> StreamRegistry::remove_stream(const std::string uid)
{
  {
    std::lock_guard<std::mutex> lock(stream_mutex_);
    auto it = stream_map_.find(uid);
  
    if (it == stream_map_.end())
    {
      return MAKE_CAM_ERROR_EXPECTED(CamError::DeviceNotFound, 
        "Could not find unplugged device " + uid + " in registry");
    }
    stream_map_.erase(it);
    spdlog::info("Stream {} successfully removed from registry", uid);

    return {};
  }

}