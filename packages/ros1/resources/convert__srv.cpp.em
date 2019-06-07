// generated from soss/cpp/ros1/resources/convert__srv.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating soss/genmsg/ros1/<package>/src/msg/convert__srv__<msg>.cpp files
@#
@# Context:
@#  - spec (genmsg.SrvSpec)
@#    Parsed specification of the .msg file
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_type_components(field) (function)
@#    Produces (package, type) of field
@#  - package (string)
@#    The name of the package
@#  - type (string)
@#    The name of the message type (not including the package name prefix)
@#######################################################################

@{
cpp_srv_type = '{}::{}'.format(package, type)
cpp_request_type = cpp_srv_type + 'Request'
cpp_response_type = cpp_srv_type + 'Response'

srv_type_string = '{}/{}'.format(package, type)

namespace_parts = ['convert', package, 'srv', type]
namespace_variable = '__'.join(namespace_parts)

ros1_srv_dependency = '{}/{}.h'.format(package, type)

conversion_dependencies = {}
for component, msg in {"request": spec.request, "response": spec.response}.items():
    for field in msg.parsed_fields():
        if field.is_builtin:
            continue

        field_pkg, field_type = get_type_components(field.type)
        key = 'soss/genmsg/ros1/{}/msg/convert__msg__{}.hpp'.format(field_pkg, field_type)
        if key not in conversion_dependencies:
            conversion_dependencies[key] = set([])
        conversion_dependencies[key].add(field.name)

alphabetical_request_fields = sorted(spec.request.parsed_fields(), key=lambda x: x.name)
alphabetical_response_fields = sorted(spec.response.parsed_fields(), key=lambda x : x.name)
}@

// Include the header for the generic message type
#include <soss/utilities.hpp>

// Include the header for the concrete service type
#include <@(ros1_srv_dependency)>

// Include the headers for the soss message dependencies
@[for key in sorted(conversion_dependencies.keys())]@
#include <@(key)> // @(', '.join(conversion_dependencies[key]))
@[end for]@

// Include the Factory header so we can add this message type to the Factory
#include <soss/ros1/Factory.hpp>

// Include the NodeHandle API so we can provide and request services
#include <ros/node_handle.h>

// Include the STL API for std::future
#include <future>

// Include the STL API for std::unordered_set
#include <unordered_set>

namespace soss {
namespace ros1 {
namespace @(namespace_variable) {

using Ros1_Srv = @(cpp_srv_type);
using Ros1_Request = @(cpp_request_type);
using Ros1_Response = @(cpp_response_type);
const std::string g_srv_name = "@(srv_type_string)";
const std::string g_request_name = g_srv_name + ":request";
const std::string g_response_name = g_srv_name + ":response";

namespace {

//==============================================================================
soss::Message initialize_request()
{
  soss::Message msg;
  msg.type = g_request_name;
@[for field in alphabetical_request_fields]@
  soss::Convert<Ros1_Request::_@(field.name)_type>::add_field(msg, "@(field.name)");
@[end for]@

  return msg;
}

//==============================================================================
void request_to_ros1(const soss::Message& from, Ros1_Request& to)
{
  auto from_field = from.data.begin();
@[for field in alphabetical_request_fields]@
  soss::Convert<Ros1_Request::_@(field.name)_type>::from_soss_field(from_field++, to.@(field.name));
@[end for]@

  // Suppress possible unused variable warnings
  (void)from;
  (void)to;
  (void)from_field;
}

//==============================================================================
void request_to_soss(const Ros1_Request& from, soss::Message& to)
{
  auto to_field = to.data.begin();
@[for field in alphabetical_request_fields]@
  soss::Convert<Ros1_Request::_@(field.name)_type>::to_soss_field(from.@(field.name), to_field++);
@[end for]@

  (void)from;
  (void)to;
  (void)to_field;
}

//==============================================================================
soss::Message initialize_response()
{
  soss::Message msg;
  msg.type = g_response_name;
@[for field in alphabetical_response_fields]@
  soss::Convert<Ros1_Response::_@(field.name)_type>::add_field(msg, "@(field.name)");
@[end for]@

  return msg;
}

//==============================================================================
void response_to_ros1(const soss::Message& from, Ros1_Response& to)
{
  auto from_field = from.data.begin();
@[for field in alphabetical_response_fields]@
  soss::Convert<Ros1_Response::_@(field.name)_type>::from_soss_field(from_field++, to.@(field.name));
@[end for]@

  (void)from;
  (void)to;
  (void)from_field;
}

//==============================================================================
void response_to_soss(const Ros1_Response& from, soss::Message& to)
{
  auto to_field = to.data.begin();
@[for field in alphabetical_response_fields]@
  soss::Convert<Ros1_Response::_@(field.name)_type>::to_soss_field(from.@(field.name), to_field++);
@[end for]@

  (void)from;
  (void)to;
  (void)to_field;
}

} // anonymous namespace

//==============================================================================
class ClientProxy final : public virtual soss::ServiceClient
{
public:

  ClientProxy(
      ros::NodeHandle& node,
      const std::string& service_name,
      const ServiceClientSystem::RequestCallback& callback)
    : _callback(callback),
      _handle(std::make_shared<PromiseHolder>())
  {
    _request = initialize_request();

    _service = node.advertiseService(
        service_name, &ClientProxy::service_callback, this);
  }

  void receive_response(
      std::shared_ptr<void> call_handle,
      const Message& result) override
  {
    const std::shared_ptr<PromiseHolder>& handle =
        std::static_pointer_cast<PromiseHolder>(call_handle);

    response_to_ros1(result, _response);
    handle->promise->set_value(_response);
  }

private:

  bool service_callback(
      Ros1_Request& request,
      Ros1_Response& response)
  {
    request_to_soss(request, _request);

    std::promise<Ros1_Response> response_promise;
    _handle->promise = &response_promise;

    std::future<Ros1_Response> future_response = response_promise.get_future();
    _callback(_request, *this, _handle);

    future_response.wait();

    response = future_response.get();

    return true;
  }

  struct PromiseHolder
  {
    std::promise<Ros1_Response>* promise;
  };

  const ServiceClientSystem::RequestCallback _callback;
  const std::shared_ptr<PromiseHolder> _handle;
  soss::Message _request;
  Ros1_Response _response;
  ros::ServiceServer _service;

};

//==============================================================================
std::shared_ptr<soss::ServiceClient> make_client(
    ros::NodeHandle& node,
    const std::string& service_name,
    const ServiceClientSystem::RequestCallback& callback)
{
  return std::make_shared<ClientProxy>(node, service_name, callback);
}

namespace {
ServiceClientFactoryRegistrar register_client(g_srv_name, &make_client);
} // anonymous namespace

class ServerProxy;
class ServerWorker;
using ServerWorkerPtr = std::shared_ptr<ServerWorker>;

//==============================================================================
struct ServerWorkContext
{
  Ros1_Request request;
  bool run;
  bool quit;

  soss::ServiceClient* soss_client;
  std::shared_ptr<void> call_handle;
};

//==============================================================================
void server_worker_thread(
    ros::NodeHandle& node,
    std::string service_name,
    ServerProxy* const owner,
    const ServerWorkerPtr worker,
    std::mutex& mutex,
    std::condition_variable& cv,
    ServerWorkContext& context);

//==============================================================================
class ServerWorker : public std::enable_shared_from_this<ServerWorker>
{
public:

  ServerWorker(
      ros::NodeHandle& node,
      const std::string& service_name,
      ServerProxy* owner)
    : _node(node),
      _service_name(service_name),
      _owner(owner)
  {
    // Do nothing. The _thread cannot be instantiated here because its
    // constructor requires the std::shared_ptr of this object, but
    // shared_from_this() cannot be called until this constructor is finished.
  }

  ServerWorkerPtr deploy(
        const soss::Message& request,
        soss::ServiceClient& soss_client,
        std::shared_ptr<void> call_handle)
  {
    if(!_thread)
    {
      // The first time this worker is deployed, we need to create the thread
      _thread = std::make_unique<std::thread>([&](){
            server_worker_thread(
                  _node, _service_name, _owner,
                  shared_from_this(), _mutex, _cv, _context);
      });
    }

    {
      // Wait until the thread is not running so we can safely modify _context
      std::unique_lock<std::mutex> lock(_mutex);

      // Update the request
      request_to_ros1(request, _context.request);

      // Update the client info
      _context.soss_client = &soss_client;
      _context.call_handle = std::move(call_handle);

      // Instruct the thread to run
      _context.run = true;
    }

    _cv.notify_one();

    return shared_from_this();
  }

  ~ServerWorker()
  {
    // Have the working thread quit
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _context.quit = true;
    }

    _cv.notify_one();
  }

private:

  ros::NodeHandle& _node;

  const std::string _service_name;

  ServerProxy* const _owner;

  std::unique_ptr<std::thread> _thread;

  std::mutex _mutex;

  std::condition_variable _cv;

  ServerWorkContext _context;
};

//==============================================================================
class ServerProxy final : public virtual soss::ServiceProvider
{
public:

  ServerProxy(
      ros::NodeHandle& node,
      const std::string& service_name)
      // Initialize the pool with 0 workers because we haven't given it the
      // desired initialization function yet
    : _node(&node),
      _workers(0)
  {
    _workers.setInitializer([=](){
      return std::make_shared<ServerWorker>(*_node, service_name, this);
    });
  }

  void call_service(
      const soss::Message& request,
      ServiceClient& soss_client,
      std::shared_ptr<void> call_handle) override
  {
    _active.insert(_workers.pop()->deploy(request, soss_client, std::move(call_handle)));
  }

  void recycle_worker(ServerWorkerPtr ptr)
  {
    _active.erase(ptr);
    _workers.recycle(std::move(ptr));
  }

private:

  // Keep the node pointer so we can spawn more ServerWorkers when needed
  ros::NodeHandle* _node;

  soss::ResourcePool<ServerWorkerPtr, &initialize_shared_null<ServerWorker>> _workers;

  std::unordered_set<ServerWorkerPtr> _active;

};

//==============================================================================
void server_worker_thread(
    ros::NodeHandle& node,
    std::string service_name,
    ServerProxy* const owner,
    const ServerWorkerPtr worker,
    std::mutex& mutex,
    std::condition_variable& cv,
    ServerWorkContext& context)
{
  ros::ServiceClient ros1_client = node.serviceClient<Ros1_Srv>(service_name);
  Ros1_Response ros1_response;

  soss::Message soss_response = initialize_response();

  while(true)
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&](){ return context.run || context.quit; });

    if(context.quit)
      return;

    ros1_client.call(context.request, ros1_response);

    response_to_soss(ros1_response, soss_response);

    context.soss_client->receive_response(std::move(context.call_handle), soss_response);

    context.run = false;
    owner->recycle_worker(worker);
  }
}

//==============================================================================
std::shared_ptr<soss::ServiceProvider> make_server(
    ros::NodeHandle& node,
    const std::string& service_name)
{
  return std::make_shared<ServerProxy>(node, service_name);
}

namespace {
ServiceProviderFactoryRegistrar register_server(g_srv_name, &make_server);
}

} // @(namespace_variable)
} // namespace ros1
} // namespace soss
