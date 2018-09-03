#include "IosxrTelemetrySub.h" 
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::CompletionQueue;
using grpc::Status;


namespace iosxr {

template<typename FwdIterator>
void deleter(FwdIterator from, FwdIterator to)
{
   while ( from != to ) 
   {
       delete *from;
       from++;
   }
}

TelemetryStream::TelemetryStream(std::shared_ptr<grpc::Channel> channel)
        : channel_(channel), stub_(IOSXRExtensibleManagabilityService::gRPCConfigOper::NewStub(channel)) {}

TelemetryStream::~TelemetryStream()
{
   deleter(callvector_.begin(), callvector_.end());
   callvector_.clear();
}

void
TelemetryStream::SubscribeAll()
{
    for (const auto& subscribe_data : subscription_set)
    {
        this->Subscribe(subscribe_data);
    }
}


// Assembles the client's payload and sends it to the server.

void 
TelemetryStream::Subscribe(const SubscriptionData& subscription_data)
{

    IOSXRExtensibleManagabilityService::CreateSubsArgs sub_args;

    sub_args.set_subidstr(subscription_data.subscription);
    sub_args.set_reqid(subscription_data.req_id);
    sub_args.set_encode(subscription_data.encoding);

    // Typically when using the asynchronous API, we hold on to the 
    //"call" instance in order to get updates on the ongoing RPC.
    // In our case it isn't really necessary, since we operate within the
    // context of the same class, but anyway, we pass it in as the tag

    callvector_.push_back(new AsyncClientCall(channel_));

    callvector_.back()->context.AddMetadata("username", this->GetCredentials()["username"]);
    callvector_.back()->context.AddMetadata("password", this->GetCredentials()["password"]);
    callvector_.back()->response_reader = 
    stub_->AsyncCreateSubs(&(callvector_.back()->context), sub_args, &cq_, (void *)callvector_.back());
}

void 
TelemetryStream::Shutdown() 
{
    tear_down = true;

    std::unique_lock<std::mutex> channel_lock(channel_mutex);

    while(!channel_closed) {
        channel_condVar.wait(channel_lock);
    }
}


void 
TelemetryStream::Cleanup() 
{
    VLOG(1) << "Asynchronous client shutdown requested"
            << "Let's clean up!";

    for (const auto& call: callvector_)
    {
        // Finish the Async session
        call->HandleResponse(false, &cq_);

        // Shutdown the completion queue
        call->HandleResponse(false, &cq_); 
    }

    VLOG(1) << "Shutting down the completion queue";
    cq_.Shutdown();

    VLOG(1) << "Notifying channel close";
    channel_closed = true;
    // Notify the condition variable;
    channel_condVar.notify_one();
}


// Loop while listening for completed responses.
// Prints out the response from the server.
void 
TelemetryStream::AsyncCompleteRpc() 
{
    void* got_tag;
    bool ok = false;
    // Storage for the status of the RPC upon completion.
    grpc::Status status;

    // Lock the mutex before notifying using the conditional variable
    std::lock_guard<std::mutex> guard(channel_mutex);


    unsigned int timeout = 5;

    // Set timeout for API
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::seconds(timeout);

    while (!tear_down) {
        auto nextStatus = cq_.AsyncNext(&got_tag, &ok, deadline);
        auto call = reinterpret_cast<AsyncClientCall*>(got_tag);
        switch(nextStatus) {
        case grpc::CompletionQueue::GOT_EVENT:
             // Verify that the request was completed successfully. Note that "ok"
             // corresponds solely to the request for updates introduced by Finish().
             call->HandleResponse(ok, &cq_);
             break;
        case grpc::CompletionQueue::SHUTDOWN:
             VLOG(1) << "Shutdown event received for completion queue";
             channel_closed = true;
             // Notify the condition variable;
             channel_condVar.notify_one();
             tear_down = true;
             break;
        case grpc::CompletionQueue::TIMEOUT:
             continue;
             break;
        }
    }

    if(!channel_closed) {
        Cleanup();
    }
}

TelemetryStream::AsyncClientCall::AsyncClientCall(std::shared_ptr<grpc::Channel> channel)
  : callStatus_(CREATE),
    telemetryDecode_(std::make_unique<TelemetryDecode>(channel)) {}

TelemetryStream::AsyncClientCall::~AsyncClientCall() 
{
    LOG(INFO) << "Call Object: " << (void *)this <<" deleted";
}

void 
TelemetryStream::AsyncClientCall::HandleResponse(bool responseStatus, 
                                                 grpc::CompletionQueue* pcq_)
{
    //The First completion queue entry indicates session creation and shouldn't be processed - Check?
    switch (callStatus_) {
    case CREATE:
        if (responseStatus) {
            response_reader->Read(&createSubsReply, (void*)this);
            if (!createSubsReply.errors().empty()) {
                LOG(ERROR) << "Error while Setting up Dial-in Connection";
                LOG(ERROR) << createSubsReply.errors();
                response_reader->Finish(&status, (void*)this);
                callStatus_ = FINISH;
            }

            LOG(INFO) << "Initial Connection to gRPC server successful: \n" 
                    << gpbMsgToJson(createSubsReply);

            callStatus_ = PROCESS;
        } else {
            response_reader->Finish(&status, (void*)this);
            callStatus_ = FINISH;
        }
        break;
    case PROCESS:
        if (responseStatus) {
            response_reader->Read(&createSubsReply, (void *)this);

            if (!createSubsReply.errors().empty()) {
                LOG(ERROR) << "Failed to Subscribe to Telemetry Stream  ";
                LOG(ERROR) << "Error: " << createSubsReply.errors();
                response_reader->Finish(&status, (void*)this);
                callStatus_ = FINISH;
            }

            VLOG(3) << "Received Subscription Reply: \n"
                    << gpbMsgToJson(createSubsReply);


            auto telemetry_data = telemetry::Telemetry();    
            telemetry_data.ParseFromString(createSubsReply.data());

            // Decode Telemetry data coming in

            telemetryDecode_->DecodeTelemetryData(telemetry_data);
                         
        } else {
            response_reader->Finish(&status, (void*)this);
            callStatus_ = FINISH;
        }

        break;
    case FINISH:
        if (status.ok()) {
             VLOG(3)  << "Server Response Completed: "  
                    << this << " CallData: " 
                    << this;
        }
        else {
            LOG(ERROR) << "RPC failed";
        }
    }
}

}
