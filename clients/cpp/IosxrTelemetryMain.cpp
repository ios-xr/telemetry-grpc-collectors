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

bool sighandle_initiated = false;

void 
signalHandler(int signum)
{

   if (!sighandle_initiated) {
       sighandle_initiated = true;
       VLOG(1) << "Interrupt signal (" << signum << ") received.";

       // Shutdown the Telemetry Async Notification Channel  
       asynchandler_telemetry_signum->Shutdown();

    } 
}

int main(int argc, char** argv) {
   
    auto server_ip = getEnvVar("SERVER_IP");
    auto server_port = getEnvVar("SERVER_PORT");
    
    auto xr_user=getEnvVar("XR_USER");
    auto xr_passwd=getEnvVar("XR_PASSWORD");

    if (server_ip == "" || server_port == "" || xr_user == "" || xr_passwd == "" ) {
        if (server_ip == "") {
            LOG(ERROR) << "SERVER_IP environment variable not set";
        }
        if (server_port == "") {
            LOG(ERROR) << "SERVER_PORT environment variable not set";
        }
        if (xr_user == "") {
            LOG(ERROR) << "XR_USER environment variable not set";
        }
        if (xr_passwd == "") {
            LOG(ERROR) << "XR_PASSWORD environment variable not set";
        }
        return 1;

    }


    std::string grpc_server = server_ip + ":" + server_port;
    auto channel = grpc::CreateChannel(
                             grpc_server, grpc::InsecureChannelCredentials());


   LOG(INFO) << "Connecting to IOS-XR gRPC server at " << grpc_server;


    // Start the Telemetry stream
    TelemetryStream telem_asynchandler(channel);

    // Spawn reader thread that maintains our Notification Channel
    std::thread telemetry_thread_ = std::thread(&TelemetryStream::AsyncCompleteRpc, &telem_asynchandler);


    telem_asynchandler.SetCredentials(xr_user, xr_passwd);

    telem_asynchandler.AddSubscription(99,
                                 IOSXR_TELEMETRY_DIALIN_GPB,
                                 "IPV6");

    telem_asynchandler.SubscribeAll();

    asynchandler_telemetry_signum = &telem_asynchandler;

    signal(SIGINT, signalHandler);  
    LOG(INFO) << "Press control-c to quit";
    telemetry_thread_.join();
    return 0;
}
