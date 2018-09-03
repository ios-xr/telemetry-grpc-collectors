#include "IosxrTelemetrySub.h"
#include <csignal>

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CompletionQueue;
using grpc::Status;
using namespace iosxr;

std::string 
getEnvVar(std::string const & key)
{
    char * val = std::getenv( key.c_str() );
    return val == NULL ? std::string("") : std::string(val);
}


TelemetryStream* asynchandler_telemetry_signum;
IosxrslVrf* vrfhandler_signum;
AsyncNotifChannel* asynchandler_slapi_signum;

bool sighandle_initiated = false;

void 
signalHandler(int signum)
{

   if (!sighandle_initiated) {
       sighandle_initiated = true;
       VLOG(1) << "Interrupt signal (" << signum << ") received.";

       // Clear out the last vrfRegMsg batch
       vrfhandler_signum->vrf_msg.clear_vrfregmsgs();

       // Create a fresh SLVrfRegMsg batch for cleanup
       vrfhandler_signum->vrfRegMsgAdd("default");

       vrfhandler_signum->unregisterVrf(AF_INET);
       vrfhandler_signum->unregisterVrf(AF_INET6);

       // Shutdown the Telemetry Async Notification Channel  
       asynchandler_telemetry_signum->Shutdown();
       // Shutdown the SLAPI Async Notification Channel  
       asynchandler_slapi_signum->Shutdown();


       //terminate program  
       //exit(signum);  
    } 
}

int main(int argc, char** argv) {
   
    auto server_ip = getEnvVar("SERVER_IP");
    auto server_port = getEnvVar("SERVER_PORT");

    if (server_ip == "" || server_port == "") {
        if (server_ip == "") {
            LOG(ERROR) << "SERVER_IP environment variable not set";
        }
        if (server_port == "") {
            LOG(ERROR) << "SERVER_PORT environment variable not set";
        }
        return 1;

    }


    std::string grpc_server = server_ip + ":" + server_port;
    auto channel = grpc::CreateChannel(
                             grpc_server, grpc::InsecureChannelCredentials());


   LOG(INFO) << "Connecting IOS-XR to gRPC server at " << grpc_server;


    AsyncNotifChannel slapi_asynchandler(channel);

    // Acquire the lock
    std::unique_lock<std::mutex> initlock(init_mutex);

    // Spawn reader thread that maintains our Notification Channel
    std::thread slapi_thread_ = std::thread(&AsyncNotifChannel::AsyncCompleteRpc, &slapi_asynchandler);


    service_layer::SLInitMsg init_msg;
    init_msg.set_majorver(service_layer::SL_MAJOR_VERSION);
    init_msg.set_minorver(service_layer::SL_MINOR_VERSION);
    init_msg.set_subver(service_layer::SL_SUB_VERSION);


    slapi_asynchandler.SendInitMsg(init_msg);

    // Wait on the mutex lock
    while (!init_success) {
        init_condVar.wait(initlock);
    }

    // Set up a new channel for vrf/route messages

    IosxrslVrf vrfhandler(grpc::CreateChannel(
                              grpc_server, grpc::InsecureChannelCredentials()));

    // Create a new SLVrfRegMsg batch
    vrfhandler.vrfRegMsgAdd("default", 10, 500);

    // Register the SLVrfRegMsg batch for v4 and v6
    vrfhandler.registerVrf(AF_INET);
    vrfhandler.registerVrf(AF_INET6);



 
    // Start the Telemetry stream
    TelemetryStream telem_asynchandler(channel);

    // Spawn reader thread that maintains our Notification Channel
    std::thread telemetry_thread_ = std::thread(&TelemetryStream::AsyncCompleteRpc, &telem_asynchandler);



    telem_asynchandler.SetCredentials("vagrant", "vagrant");

    telem_asynchandler.AddSubscription(99,
                                 IOSXR_TELEMETRY_DIALIN_GPB,
                                 "IPV6");

    //asynchandler.AddSubscription(99,
    //                             IOSXR_TELEMETRY_DIALIN_GPB,
    //                            "INTERFACESSUB");

    telem_asynchandler.SubscribeAll();

    asynchandler_telemetry_signum = &telem_asynchandler;
    vrfhandler_signum = &vrfhandler;
    asynchandler_slapi_signum = &slapi_asynchandler;

    signal(SIGINT, signalHandler);  
    LOG(INFO) << "Press control-c to quit";
    telemetry_thread_.join();
    slapi_thread_.join();
    return 0;
}
