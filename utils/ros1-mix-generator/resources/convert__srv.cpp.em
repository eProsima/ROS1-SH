// generated from is-ros1/resources/convert__srv.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating is/genmsg/ros1/<package>/src/msg/convert__srv__<msg>.cpp files
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
BUILTIN_TYPES = {
    'bool'    : 'bool',
    'byte'    : 'int8_t',
    'char'    : 'char',
    'wchar'   : 'wchar_t',
    'float32' : 'float',
    'float64' : 'double',
    'float128': 'long double',
    'int8'    : 'int8_t',
    'uint8'   : 'uint8_t',
    'int16'   : 'int16_t',
    'uint16'  : 'uint16_t',
    'int32'   : 'int32_t',
    'uint32'  : 'uint32_t',
    'int64'   : 'int64_t',
    'uint64'  : 'uint64_t'
}

cpp_srv_type = '{}::{}'.format(package, type)
cpp_request_type = cpp_srv_type + 'Request'
cpp_response_type = cpp_srv_type + 'Response'

srv_type_string = '{}/{}'.format(package, type)

namespace_parts_srv = ['convert', package, 'srv', type]
namespace_variable_srv = '__'.join(namespace_parts_srv)

ros1_srv_dependency = '{}/{}.h'.format(package, type)

conversion_dependencies = {}
for component, msg in {"request": spec.request, "response": spec.response}.items():
    for field in msg.parsed_fields():
        if field.is_builtin:
            continue

        field_pkg, field_type = get_type_components(field.type)
        key = 'is/genmsg/ros1/{}/msg/convert__msg__{}.hpp'.format(field_pkg, field_type)
        if key not in conversion_dependencies:
            conversion_dependencies[key] = set([])
        conversion_dependencies[key].add(field.name)
}@

// Include the header for the generic message type
#include <is/core/Message.hpp>

// Include the header for the conversions
#include <is/utils/Convert.hpp>

// Include the header for the logger
#include <is/utils/Log.hpp>

// Include the header for the concrete service type
#include <@(ros1_srv_dependency)>

// Include the headers for the Integration Service message dependencies
@[for key in sorted(conversion_dependencies.keys())]@
#include <@(key)> // @(', '.join(conversion_dependencies[key]))
@[end for]@

// Include the Factory header so we can add this message type to the Factory
#include <is/sh/ros1/Factory.hpp>

// Include the NodeHandle API so we can provide and request services
#include <ros/node_handle.h>

// Include the STL API for std::future
#include <future>

// Include the STL API for std::unordered_set
#include <unordered_set>

namespace eprosima {
namespace is {
namespace sh {
namespace ros1 {
namespace @(namespace_variable_srv) {

static eprosima::is::utils::Logger logger ("is::sh::ROS1");

using Ros1_Srv = @(cpp_srv_type);
using Ros1_Request = @(cpp_request_type);
using Ros1_Response = @(cpp_response_type);
const std::string g_srv_name = "@(srv_type_string)";
const std::string g_request_name = g_srv_name + ":request";
const std::string g_response_name = g_srv_name + ":response";

namespace {
@[for component, fields in {'request' : spec.request.parsed_fields(), 'response' : spec.response.parsed_fields()}.items()]@
inline const xtypes::StructType @(component)_type()
{
    xtypes::StructType type(g_@(component)_name);
@[    if not fields]@
    type.add_member("structure_needs_at_least_one_member", xtypes::primitive_type<bool>());
@[    else]@
@[        for field in fields]@
@[            if field.base_type in BUILTIN_TYPES.keys()]@
    const xtypes::DynamicType& derived_type_@(field.name) = xtypes::primitive_type<@(BUILTIN_TYPES[field.base_type])>();
@[            else]@
@[                if 'string' in field.base_type]@
    const xtypes::StringType derived_type_@(field.name) = xtypes::StringType();
@[                elif field.base_type in ['duration', 'time']]@
    const xtypes::StructType derived_type_@(field.name) ( // Special "@(field.base_type)" ROS1 built-in type
        is::sh::ros1::convert__msg__Timebase::type("@(field.name)"));
@[                else]@
    const xtypes::StructType derived_type_@(field.name) (
        is::sh::ros1::convert__@(field.base_type[:field.base_type.find('/')])__msg__@(field.base_type[field.base_type.find('/')+1:])::type());
@[                end if]@
@[            end if]@
@[            if field.is_array]@
@[                if field.array_len]@
    type.add_member("@(field.name)", xtypes::SequenceType(std::move(derived_type_@(field.name)), @(field.array_len)));
@[                else]@
    type.add_member("@(field.name)", xtypes::SequenceType(std::move(derived_type_@(field.name))));
@[                end if]@
@[            else]@
    type.add_member("@(field.name)", std::move(derived_type_@(field.name)));
@[            end if]@
@[        end for]@
@[    end if]@
    return type;
}

TypeToFactoryRegistrar register_@(component)_type(g_@(component)_name, &@(component)_type);

@[end for]@
} //  anonymous namespace

//==============================================================================
void request_to_ros1(const xtypes::ReadableDynamicDataRef& from, Ros1_Request& to)
{
@[for field in spec.request.parsed_fields()]@
    is::utils::Convert<Ros1_Request::_@(field.name)_type>::from_xtype_field(from["@(field.name)"], to.@(field.name));
@[end for]@

    // Suppress possible unused variable warnings
    (void)from;
    (void)to;
}

//==============================================================================
void request_to_xtype(const Ros1_Request& from, xtypes::WritableDynamicDataRef to)
{
@[for field in spec.request.parsed_fields()]@
    is::utils::Convert<Ros1_Request::_@(field.name)_type>::to_xtype_field(from.@(field.name), to["@(field.name)"]);
@[end for]@

    (void)from;
    (void)to;
}

//==============================================================================
void response_to_ros1(const xtypes::ReadableDynamicDataRef& from, Ros1_Response& to)
{
@[for field in spec.response.parsed_fields()]@
    is::utils::Convert<Ros1_Response::_@(field.name)_type>::from_xtype_field(from["@(field.name)"], to.@(field.name));
@[end for]@

    (void)from;
    (void)to;
}

//==============================================================================
void response_to_xtype(const Ros1_Response& from, xtypes::WritableDynamicDataRef to)
{
@[for field in spec.response.parsed_fields()]@
    is::utils::Convert<Ros1_Response::_@(field.name)_type>::to_xtype_field(from.@(field.name), to["@(field.name)"]);
@[end for]@

    (void)from;
    (void)to;
}

//==============================================================================
class ClientProxy final : public virtual is::ServiceClient
{
public:

    ClientProxy(
            ros::NodeHandle& node,
            const std::string& service_name,
            ServiceClientSystem::RequestCallback* callback)
        : _callback(callback)
        , _handle(std::make_shared<PromiseHolder>())
        , _request_type(request_type())
        , _request_data(*_request_type)
        , _service_name(service_name)
    {
        _service = node.advertiseService(
            service_name, &ClientProxy::service_callback, this);
    }

    void receive_response(
            std::shared_ptr<void> call_handle,
            const xtypes::DynamicData& result) override
    {
        const std::shared_ptr<PromiseHolder>& handle =
                std::static_pointer_cast<PromiseHolder>(call_handle);

        response_to_ros1(result, _response);

        logger << utils::Logger::Level::INFO
               << "Translating reply from Integration Service to ROS 1 for service reply topic '"
               << _service_name << "_Reply': [[ " << _response << " ]]" << std::endl;

        handle->promise->set_value(_response);
    }

private:

    bool service_callback(
            Ros1_Request& request,
            Ros1_Response& response)
    {

        logger << utils::Logger::Level::INFO
               << "Receiving request from ROS 1 for service request topic '"
               << _service_name << "_Request'" << std::endl;

        request_to_xtype(request, _request_data);

        std::promise<Ros1_Response> response_promise;
        _handle->promise = &response_promise;

        std::future<Ros1_Response> future_response = response_promise.get_future();
        (*_callback)(_request_data, *this, _handle);

        future_response.wait();

        response = future_response.get();

        return true;
    }

    struct PromiseHolder
    {
        std::promise<Ros1_Response>* promise;
    };

    ServiceClientSystem::RequestCallback* _callback;
    const std::shared_ptr<PromiseHolder> _handle;
    std::string _service_name;
    const xtypes::DynamicType::Ptr _request_type;
    xtypes::DynamicData _request_data;
    Ros1_Response _response;
    ros::ServiceServer _service;

};

//==============================================================================
std::shared_ptr<is::ServiceClient> make_client(
        ros::NodeHandle& node,
        const std::string& service_name,
        ServiceClientSystem::RequestCallback* callback)
{
    return std::make_shared<ClientProxy>(node, service_name, callback);
}

namespace {
ServiceClientToFactoryRegistrar register_client(g_response_name, &make_client);
} //  anonymous namespace

class ServerProxy;
class ServerWorker;
using ServerWorkerPtr = std::shared_ptr<ServerWorker>;

//==============================================================================
struct ServerWorkContext
{
    ServerWorkContext()
        : run(false)
        , quit(false)
    {
    }

    Ros1_Request request;
    bool run;
    bool quit;

    is::ServiceClient* is_client;
    std::shared_ptr<void> call_handle;
};

//==============================================================================
void server_worker_thread(
        ros::NodeHandle& node,
        std::string service_name,
        const xtypes::DynamicType::Ptr response_type,
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
        : _node(node)
        , _service_name(service_name)
        , _response_type(response_type())
        , _owner(owner)
    {
        // Do nothing. The _thread cannot be instantiated here because its
        // constructor requires the std::shared_ptr of this object, but
        // shared_from_this() cannot be called until this constructor is finished.
    }

    ServerWorkerPtr deploy(
            const xtypes::DynamicData& request,
            is::ServiceClient& is_client,
            std::shared_ptr<void> call_handle)
    {
        if (!_thread)
        {
            // The first time this worker is deployed, we need to create the thread
            _thread = std::make_unique<std::thread>([&]()
                            {
                                server_worker_thread(
                                    _node, _service_name, _response_type, _owner,
                                    shared_from_this(), _mutex, _cv, _context);
                            });
        }

        {
            // Wait until the thread is not running so we can safely modify _context
            std::unique_lock<std::mutex> lock(_mutex);

            // Update the request
            request_to_ros1(request, _context.request);

            // Update the client info
            _context.is_client = &is_client;
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

    const xtypes::DynamicType::Ptr _response_type;

    ServerProxy* const _owner;

    std::unique_ptr<std::thread> _thread;

    std::mutex _mutex;

    std::condition_variable _cv;

    ServerWorkContext _context;
};

//==============================================================================
class ServerProxy final : public virtual is::ServiceProvider
{
public:

    ServerProxy(
            ros::NodeHandle& node,
            const std::string& service_name)
    // Initialize the pool with 0 workers because we haven't given it the
    // desired initialization function yet
        : _node(&node)
        , _workers(0)
        , _service_name(service_name)
    {
        _workers.setInitializer([=]()
                {
                    return std::make_shared<ServerWorker>(*_node, service_name, this);
                });
    }

    void call_service(
            const xtypes::DynamicData& request,
            ServiceClient& is_client,
            std::shared_ptr<void> call_handle) override
    {
        logger << utils::Logger::Level::INFO
               << "Translating request from Integration Service to ROS 1 for service request topic '"
               << _service_name << "_Request': [[ " << request << " ]]" << std::endl;

        _active.insert(_workers.pop()->deploy(request, is_client, std::move(call_handle)));
    }

    void recycle_worker(
            ServerWorkerPtr ptr)
    {
        _active.erase(ptr);
        _workers.recycle(std::move(ptr));
    }

private:

    // Keep the node pointer so we can spawn more ServerWorkers when needed
    ros::NodeHandle* _node;

    is::utils::ResourcePool<ServerWorkerPtr, &is::utils::initialize_shared_null<ServerWorker> > _workers;

    std::unordered_set<ServerWorkerPtr> _active;

    std::string _service_name;

};

//==============================================================================
void server_worker_thread(
        ros::NodeHandle& node,
        std::string service_name,
        const xtypes::DynamicType::Ptr response_type,
        ServerProxy* const owner,
        const ServerWorkerPtr worker,
        std::mutex& mutex,
        std::condition_variable& cv,
        ServerWorkContext& context)
{
    ros::ServiceClient ros1_client = node.serviceClient<Ros1_Srv>(service_name);
    Ros1_Response ros1_response;

    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&]()
                {
                    return context.run || context.quit;
                });

        if (context.quit)
        {
            return;
        }

        xtypes::DynamicData response(*response_type);

        ros1_client.call(context.request, ros1_response);

        response_to_xtype(ros1_response, response);

        context.is_client->receive_response(std::move(context.call_handle), response);

        context.run = false;
        owner->recycle_worker(worker);
    }
}

//==============================================================================
std::shared_ptr<is::ServiceProvider> make_server(
        ros::NodeHandle& node,
        const std::string& service_name)
{
    return std::make_shared<ServerProxy>(node, service_name);
}

namespace {
ServiceProviderToFactoryRegistrar register_server(g_request_name, &make_server);
} // namespace {

} //  @(namespace_variable_srv)
} //  namespace ros1
} //  namespace sh
} //  namespace is
} //  namespace eprosima
