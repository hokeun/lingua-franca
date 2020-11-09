// Code generated by the Lingua Franca compiler from file:
// /Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf
#include "ctarget.h"
#define NUMBER_OF_FEDERATES 1
#include "core/reactor_threaded.c"
// =============== START reactor class MQTTSubscriber
// *********** From the preamble, verbatim:
#line 66 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
#include "MQTTClient.h"
#include "util.h"
#include <pthread.h>

#define QOS         2
#define TIMEOUT     10000L

// Connection options for the client.
// Making this global means that all instances of this reactor have
// the same connection options.
MQTTClient_connectOptions sub_connect_options = MQTTClient_connectOptions_initializer;
        
// Callback function invoked by MQTT when a message arrives.
int message_arrived(
    void *incoming_message, 
    char *topicName, 
    int topicLen, 
    MQTTClient_message *message
) {
    instant_t receive_physical_time = get_physical_time();
    // If a physical timestamp was sent, report the transport time.
    size_t string_length = strlen((char*)message->payload); // Assumes null-terminated string.
    if (message->payloadlen == string_length + 1 + 2*sizeof(instant_t)) {
        instant_t* physical_timestamp = (instant_t*)((char*)message->payload + string_length + 1);
        // printf("DEBUG: MQTTReceiver.message_arrived: Received message after measured latency of %lld nsec (assuming synchronized clocks).\n", receive_physical_time - *physical_timestamp);
    }
    
    // printf("DEBUG: MQTTSubscriber: Message arrived on topic %s: %s\n", topicName, (char*)message->payload);
                
    // Extract the timestamp and calculate delay from current_time to that timestamp.
    // Note that if this subscriber's current time is ahead of current time
    // at the publisher (something that should not happen even in a distributed
    // implementation), then this extra delay may be negative. If it becomes
    // less than -offset, then the schedule_copy() function will complain
    // by printing a warning to stderr and revise the timestamp to current time.
    // First acquire the mutex lock. Otherwise, logical time could have a big
    // jump between this calculation and the call to schedule, resulting in a long delay.
    
    // NOTE: Since schedule_copy also acquires this lock, we assume here that the
    // pthreads library correctly implements recursive mutex locks to unlock all
    // locks held by the current thread when waiting for signals.
    pthread_mutex_lock(&mutex);
    
    instant_t timestamp = extract_ll((unsigned char*)message->payload + message->payloadlen - sizeof(instant_t));
    interval_t delay = timestamp - get_logical_time();
    // printf("DEBUG: MQTTSubscriber.message_arrived: received timestamp that is %lld ahead of current_time %lld.\n", *timestamp - start_time, current_time);
    // printf("DEBUG: MQTTSubscriber.message_arrived: physical time is ahead of current logical time by: %lld.\n", receive_physical_time - current_time);
                
    // Schedule the event. Since incoming_message is a physical action,
    // the offset specified in the second argument will be added to current_time
    // and to the min_delay in the action and then compared to physical time.
    // If the sum is greater than physical time
    // (i.e. if the offset + min_delay is large enough), then the event will be scheduled at
    // exactly the logical time at the publisher plus the offset.
    // Otherwise, it will be scheduled at the current physical time.
    // The incoming message is in dynamically allocated memory.
    // We copy the message using schedule_copy() because, unfortunately, Paho MQTT uses its own
    // version of malloc() and free() (defined in Heap.h and Heap.c).
    // We could modify Paho MQTT to use the generic malloc() and free(),
    // and then we could use schedule_value() to avoid the copy.
    // Note that the last 8 bytes of the message are the sender's timestamp.
    // We include that in the copy so that the reaction to the physical action
    // can measure the latency.
    schedule_copy(incoming_message, delay, (char*)message->payload, message->payloadlen);
    
    pthread_mutex_unlock(&mutex);

    // MQTTClient_freeMessage() also frees the memory allocated to the payload,
    // which is why we have to copy the message here.
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    // Return true to indicate that the message has been successfully handled.
    return 1;
}

/** Callback invoked if the connection is lost. */
void sub_connection_lost(void *incoming_message, char *cause) {
    printf("\nConnection lost\n");
    printf("  cause: %s\n", cause);
}
    

// *********** End of preamble.
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
typedef struct {
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    char* value;
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    bool is_present;
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int num_destinations;
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    token_t* token;
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int length;
#line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
} mqttsubscriber_message_t;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
typedef struct {
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    trigger_t* trigger;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    char* value;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    bool is_present;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    bool has_value;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    token_t* token;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    interval_t tardiness;
#line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
} mqttsubscriber_incoming_message_t;
typedef struct {
    int bank_index;
    #line 60 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    string address;
    #line 61 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    string clientID;
    #line 62 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    string topic;
    #line 63 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    interval_t offset;
    #line 170 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    MQTTClient client;
    #line 175 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    interval_t max_latency;
    #line 180 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    interval_t latencies;
    #line 185 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int count;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    mqttsubscriber_incoming_message_t __incoming_message;
    #line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    mqttsubscriber_message_t __message;
    #line 154 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int __message__width;
    #line 187 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t ___reaction_0;
    #line 209 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t ___reaction_1;
    #line 259 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t ___reaction_2;
    trigger_t ___startup;
    reaction_t* ___startup_reactions[1];
    trigger_t ___shutdown;
    reaction_t* ___shutdown_reactions[1];
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    trigger_t ___incoming_message;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t* ___incoming_message_reactions[1];
} mqttsubscriber_self_t;
void mqttsubscriberreaction_function_0(void* instance_args) {
    mqttsubscriber_self_t* self = (mqttsubscriber_self_t*)instance_args;
    mqttsubscriber_incoming_message_t* incoming_message = &self->__incoming_message;
    #line 188 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    MQTTClient_create(&self->client, self->address, self->clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    sub_connect_options.keepAliveInterval = 20;
    sub_connect_options.cleansession = 1;
    
    // Set up callback functions.
    // Last argument should be a pointer to a function to
    // handle notification of delivery of a sent message.
    // But this reactor isn't sending any messages.
    MQTTClient_setCallbacks(self->client, incoming_message, sub_connection_lost, message_arrived, NULL);
    
    // Connect to the broker.
    int rc; // response code.
    if ((rc = MQTTClient_connect(self->client, &sub_connect_options)) != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTSubscriber: Failed to connect to MQTT broker.\n");
        fprintf(stderr, "Perhaps one is not running? Return code: %d\n", rc);
        exit(EXIT_FAILURE);
    }
    
    MQTTClient_subscribe(self->client, self->topic, QOS);
        
}
void mqttsubscriberreaction_function_1(void* instance_args) {
    mqttsubscriber_self_t* self = (mqttsubscriber_self_t*)instance_args;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    // Expose the action struct as a local variable whose name matches the action name.
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    mqttsubscriber_incoming_message_t* incoming_message = &self->__incoming_message;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    // Set the fields of the action struct to match the current trigger.
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    incoming_message->is_present = self->___incoming_message.is_present;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    incoming_message->has_value = ((self->___incoming_message.token) != NULL && (self->___incoming_message.token)->value != NULL);
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    incoming_message->token = (self->___incoming_message.token);
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    if (incoming_message->has_value) {
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
        incoming_message->value = (char*)(self->___incoming_message.token)->value;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    }
    mqttsubscriber_message_t* message = &self->__message;
    #line 210 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->count++;
    
    // The incoming_message action contains a token that we can just forward.
    // The allocated memory will be freed when the token's reference count hits 0.
    // Note that this token will still contain the sender's timestamp.
    SET_TOKEN(message, incoming_message->token);
            
    // Get the sender's timestamp.
    instant_t* timestamp = (instant_t*)(
        incoming_message->token->value + incoming_message->token->length - sizeof(instant_t)
    );
     //printf("DEBUG: Received message carrying timestamp %lld.\n", self->count, *timestamp);
    
    // If the offset is 0, then the latency will be a measure of
    // the physical latency between creation of the message at the sender
    // and its receipt here, offset by the clock synchronization error,
    // assuming that the sender sent the message at a physical time matching its
    // logical timestamp.
    instant_t logical_timestamp = get_logical_time();
    interval_t latency = logical_timestamp - *timestamp;
    // printf("DEBUG: MQTTSubscriber.reaction: Received timestamp is larger than sent timestamp by: %lld.\n", latency);
    printf("EVENT: message_id: %d\n", self->count);
    printf("EVENT: sender_lts: %lld\n", *timestamp);
    printf("EVENT: receiver_lts: %lld\n", logical_timestamp);
    printf("EVENT: logical_latency: %lld\n", latency);
    
    // If a physical timestamp was sent, use that to collect
    // latency stats instead of the logical time increment.
    size_t string_length = strlen(incoming_message->value); // Assumes null-terminated string.
    if (incoming_message->token->length == string_length + 1 + 2*sizeof(instant_t)) {
        instant_t receive_physical_time = get_physical_time();
        instant_t physical_timestamp = extract_ll((unsigned char*)(incoming_message->value + string_length + 1));
        latency = receive_physical_time - physical_timestamp;
        printf("EVENT: sender_pts: %lld\n", physical_timestamp);
        printf("EVENT: receiver_pts: %lld\n", receive_physical_time);
        printf("EVENT: physical_latency: %lld\n", latency);
        //printf("EVENT: MQTTReceiver.reaction: Reacted to message after measured latency of %lld nsec (assuming synchronized clocks).\n", latency);
    }
    else{ //physical timestamp was not sent, echo physical timestamp for dashboard
        printf("EVENT: receiver_pts: %lld\n", get_physical_time());
    }
    
    self->latencies += latency;
    
    if (latency > self->max_latency) {
        self->max_latency = latency;
    }
        
}
void mqttsubscriberreaction_function_2(void* instance_args) {
    mqttsubscriber_self_t* self = (mqttsubscriber_self_t*)instance_args;
    #line 260 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    printf("MQTTSubscriber: Maximum latency measured at receiver (in nsec): %lld.\n", self->max_latency);
    if (self->count > 0) {
        printf("MQTTSubscriber: Average latency measured at receiver (in nsec): %lld.\n", self->latencies/self->count);
    }
    printf("MQTTSubscriber: Client ID %s disconnecting.\n", self->clientID);
    MQTTClient_disconnect(self->client, 10000);
    MQTTClient_destroy(&self->client);
        
}
mqttsubscriber_self_t* new_MQTTSubscriber() {
    mqttsubscriber_self_t* self = (mqttsubscriber_self_t*)calloc(1, sizeof(mqttsubscriber_self_t));
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->__incoming_message.trigger = &self->___incoming_message;
    #line 187 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.function = mqttsubscriberreaction_function_0;
    #line 187 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.self = self;
    #line 187 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.deadline_violation_handler = NULL;
    #line 187 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.tardy_handler = NULL;
    #line 209 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_1.function = mqttsubscriberreaction_function_1;
    #line 209 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_1.self = self;
    #line 209 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_1.deadline_violation_handler = NULL;
    #line 209 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_1.tardy_handler = NULL;
    #line 259 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_2.function = mqttsubscriberreaction_function_2;
    #line 259 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_2.self = self;
    #line 259 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_2.deadline_violation_handler = NULL;
    #line 259 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_2.tardy_handler = NULL;
    self->___startup_reactions[0] = &self->___reaction_0;
    self->___startup.last = NULL;
    self->___startup.reactions = &self->___startup_reactions[0];
    self->___startup.number_of_reactions = 1;
    self->___startup.is_timer = false;
    self->___shutdown_reactions[0] = &self->___reaction_2;
    self->___shutdown.last = NULL;
    self->___shutdown.reactions = &self->___shutdown_reactions[0];
    self->___shutdown.number_of_reactions = 1;
    self->___shutdown.is_timer = false;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___incoming_message.last = NULL;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___incoming_message_reactions[0] = &self->___reaction_1;
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___incoming_message.reactions = &self->___incoming_message_reactions[0];
    #line 165 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___incoming_message.number_of_reactions = 1;
    self->___incoming_message.is_physical = true;
    self->___incoming_message.element_size = sizeof(char);
    return self;
}
void delete_MQTTSubscriber(mqttsubscriber_self_t* self) {
    if (self->___reaction_0.output_produced != NULL) {
        free(self->___reaction_0.output_produced);
    }
    if (self->___reaction_0.triggers != NULL) {
        free(self->___reaction_0.triggers);
    }
    if (self->___reaction_0.triggered_sizes != NULL) {
        free(self->___reaction_0.triggered_sizes);
    }
    if (self->___reaction_1.output_produced != NULL) {
        free(self->___reaction_1.output_produced);
    }
    if (self->___reaction_1.triggers != NULL) {
        free(self->___reaction_1.triggers);
    }
    if (self->___reaction_1.triggered_sizes != NULL) {
        free(self->___reaction_1.triggered_sizes);
    }
    if (self->___reaction_2.output_produced != NULL) {
        free(self->___reaction_2.output_produced);
    }
    if (self->___reaction_2.triggers != NULL) {
        free(self->___reaction_2.triggers);
    }
    if (self->___reaction_2.triggered_sizes != NULL) {
        free(self->___reaction_2.triggered_sizes);
    }
    for(int i = 0; i < self->___reaction_0.num_outputs; i++) {
        free(self->___reaction_0.triggers[i]);
    }
    for(int i = 0; i < self->___reaction_1.num_outputs; i++) {
        free(self->___reaction_1.triggers[i]);
    }
    for(int i = 0; i < self->___reaction_2.num_outputs; i++) {
        free(self->___reaction_2.triggers[i]);
    }
    free(self);
}
// =============== END reactor class MQTTSubscriber

// =============== START reactor class recvSpatMessage2
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
typedef struct {
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    char* value;
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    bool is_present;
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int num_destinations;
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    token_t* token;
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int length;
#line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
} recvspatmessage2_message_t;
typedef struct {
    int bank_index;
    #line 46 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int signal_phase;
    #line 47 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    interval_t residual;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    recvspatmessage2_message_t* __message;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    // width of -2 indicates that it is not a multiport.
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    int __message__width;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    // Default input (in case it does not get connected)
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    recvspatmessage2_message_t __default__message;
    #line 49 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t ___reaction_0;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    trigger_t ___message;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    reaction_t* ___message_reactions[1];
} recvspatmessage2_self_t;
void recvspatmessage2reaction_function_0(void* instance_args) {
    recvspatmessage2_self_t* self = (recvspatmessage2_self_t*)instance_args;
    recvspatmessage2_message_t* message = self->__message;
    if (message->is_present) {
        message->length = message->token->length;
        message->value = (char*)message->token->value;
    } else {
        message->length = 0;
    }
    int message_width = self->__message__width;
    #line 50 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    /*printf("PrintMessage: At (elapsed) time %lld, subscriber receives: %s\n",
                get_elapsed_logical_time(),
                message->value
            );*/
    
    sscanf(message->value, "%d %llu", &self->signal_phase, &self->residual);
    printf("EVENT: residual: %llu\n", self->residual);
    printf("EVENT: phase: %d\n", self->signal_phase);
    printf("spatReceiver: At phy time %lld, Phase:%d residual:%llu nsec.\n", get_physical_time(), self->signal_phase, self->residual);
           
}
void recvspatmessage2_deadline_function0(void* instance_args) {
    recvspatmessage2_self_t* self = (recvspatmessage2_self_t*)instance_args;
    recvspatmessage2_message_t* message = self->__message;
    if (message->is_present) {
        message->length = message->token->length;
        message->value = (char*)message->token->value;
    } else {
        message->length = 0;
    }
    int message_width = self->__message__width;
    #line 60 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    printf("EVENT: deadline_miss: 1\n");
        
}
recvspatmessage2_self_t* new_recvSpatMessage2() {
    recvspatmessage2_self_t* self = (recvspatmessage2_self_t*)calloc(1, sizeof(recvspatmessage2_self_t));
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    // Set input by default to an always absent default input.
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->__message = &self->__default__message;
    #line 49 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.function = recvspatmessage2reaction_function_0;
    #line 49 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.self = self;
    #line 49 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.deadline_violation_handler = &recvspatmessage2_deadline_function0;
    #line 49 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___reaction_0.tardy_handler = NULL;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___message.last = NULL;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___message_reactions[0] = &self->___reaction_0;
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___message.reactions = &self->___message_reactions[0];
    #line 45 "file:/Users/ra/git/lingua-franca/example/Denso/SpatAnalysis/spatReceiver.lf"
    self->___message.number_of_reactions = 1;
    self->___message.element_size = sizeof(char);
    return self;
}
void delete_recvSpatMessage2(recvspatmessage2_self_t* self) {
    if (self->___reaction_0.output_produced != NULL) {
        free(self->___reaction_0.output_produced);
    }
    if (self->___reaction_0.triggers != NULL) {
        free(self->___reaction_0.triggers);
    }
    if (self->___reaction_0.triggered_sizes != NULL) {
        free(self->___reaction_0.triggered_sizes);
    }
    for(int i = 0; i < self->___reaction_0.num_outputs; i++) {
        free(self->___reaction_0.triggers[i]);
    }
    free(self);
}
// =============== END reactor class recvSpatMessage2

// =============== START reactor class spatReceiver
typedef struct {
    int bank_index;
} spatreceiver_self_t;
spatreceiver_self_t* new_spatReceiver() {
    spatreceiver_self_t* self = (spatreceiver_self_t*)calloc(1, sizeof(spatreceiver_self_t));
    return self;
}
void delete_spatReceiver(spatreceiver_self_t* self) {
    free(self);
}
// =============== END reactor class spatReceiver

char* __default_argv[] = {"X", "-k", "true", "-o", "60", "secs"};
void __set_default_command_line_options() {
    default_argc = 6;
    default_argv = __default_argv;
}
// Array of pointers to timer triggers to be scheduled in __initialize_timers().
trigger_t** __timer_triggers = NULL;
int __timer_triggers_size = 0;
// Array of pointers to timer triggers to be scheduled in __trigger_startup_reactions().
reaction_t* __startup_reactions[1];
int __startup_reactions_size = 1;
// Array of pointers to shutdown triggers.
trigger_t* __shutdown_triggers[1];
int __shutdown_triggers_size = 1;
trigger_t* __action_for_port(int port_id) {
    return NULL;
}
void __initialize_trigger_objects() {
    __tokens_with_ref_count_size = 2;
    __tokens_with_ref_count = (token_present_t*)malloc(2 * sizeof(token_present_t));
    // Create the array that will contain pointers to is_present fields to reset on each step.
    __is_present_fields_size = 2;
    __is_present_fields = (bool**)malloc(2 * sizeof(bool*));
    // ************* Instance spatReceiver of class spatReceiver
    spatreceiver_self_t* spatreceiver_self = new_spatReceiver();
    //***** Start initializing spatReceiver
    // ************* Instance spatReceiver.rsuMsg of class recvSpatMessage2
    recvspatmessage2_self_t* spatreceiver_rsumsg_self = new_recvSpatMessage2();
    //***** Start initializing spatReceiver.rsuMsg
    // width of -2 indicates that it is not a multiport.
    spatreceiver_rsumsg_self->__message__width = -2;
    static int spatreceiver_rsumsg_initial_signal_phase = 0;
    spatreceiver_rsumsg_self->signal_phase = spatreceiver_rsumsg_initial_signal_phase;
    spatreceiver_rsumsg_self->residual = NSEC(0);
    spatreceiver_rsumsg_self->___reaction_0.deadline = MSEC(10);
    //***** End initializing spatReceiver.rsuMsg
    // ************* Instance spatReceiver.sub of class MQTTSubscriber
    mqttsubscriber_self_t* spatreceiver_sub_self = new_MQTTSubscriber();
    //***** Start initializing spatReceiver.sub
    spatreceiver_sub_self->address = "tcp://localhost:1883"; 
    spatreceiver_sub_self->clientID = "Vehicle 760"; 
    spatreceiver_sub_self->topic = "spat/rsu101"; 
    spatreceiver_sub_self->offset = MSEC(10); 
    // width of -2 indicates that it is not a multiport.
    spatreceiver_sub_self->__message__width = -2;
    __startup_reactions[0] = &spatreceiver_sub_self->___reaction_0;
    __shutdown_triggers[0] = &spatreceiver_sub_self->___shutdown;
    static MQTTClient spatreceiver_sub_initial_client = NULL;
    spatreceiver_sub_self->client = spatreceiver_sub_initial_client;
    spatreceiver_sub_self->max_latency = 0;
    spatreceiver_sub_self->latencies = 0;
    static int spatreceiver_sub_initial_count = 0;
    spatreceiver_sub_self->count = spatreceiver_sub_initial_count;
    spatreceiver_sub_self->___incoming_message.offset = MSEC(10);
    spatreceiver_sub_self->___incoming_message.period = -1;
    spatreceiver_sub_self->___incoming_message.token = __create_token(sizeof(char));
    spatreceiver_sub_self->___incoming_message.is_present = false;
    __tokens_with_ref_count[0].token
            = &spatreceiver_sub_self->___incoming_message.token;
    __tokens_with_ref_count[0].is_present
            = &spatreceiver_sub_self->___incoming_message.is_present;
    __tokens_with_ref_count[0].reset_is_present = true;
    //***** End initializing spatReceiver.sub
    //***** End initializing spatReceiver
    // Populate arrays of trigger pointers.
    spatreceiver_self->bank_index = 0;
    spatreceiver_rsumsg_self->bank_index = 0;
    // Total number of outputs produced by the reaction.
    spatreceiver_rsumsg_self->___reaction_0.num_outputs = 0;
    // Allocate arrays for triggering downstream reactions.
    if (spatreceiver_rsumsg_self->___reaction_0.num_outputs > 0) {
        spatreceiver_rsumsg_self->___reaction_0.output_produced = (bool**)malloc(sizeof(bool*) * spatreceiver_rsumsg_self->___reaction_0.num_outputs);
        spatreceiver_rsumsg_self->___reaction_0.triggers = (trigger_t***)malloc(sizeof(trigger_t**) * spatreceiver_rsumsg_self->___reaction_0.num_outputs);
        spatreceiver_rsumsg_self->___reaction_0.triggered_sizes = (int*)malloc(sizeof(int) * spatreceiver_rsumsg_self->___reaction_0.num_outputs);
    }
    // Initialize the output_produced array.
    // Reaction 0 of spatReceiver.rsuMsg depends on one maximal upstream reaction.
    spatreceiver_rsumsg_self->___reaction_0.last_enabling_reaction = &(spatreceiver_sub_self->___reaction_1);
    spatreceiver_sub_self->bank_index = 0;
    // Total number of outputs produced by the reaction.
    spatreceiver_sub_self->___reaction_0.num_outputs = 0;
    // Allocate arrays for triggering downstream reactions.
    if (spatreceiver_sub_self->___reaction_0.num_outputs > 0) {
        spatreceiver_sub_self->___reaction_0.output_produced = (bool**)malloc(sizeof(bool*) * spatreceiver_sub_self->___reaction_0.num_outputs);
        spatreceiver_sub_self->___reaction_0.triggers = (trigger_t***)malloc(sizeof(trigger_t**) * spatreceiver_sub_self->___reaction_0.num_outputs);
        spatreceiver_sub_self->___reaction_0.triggered_sizes = (int*)malloc(sizeof(int) * spatreceiver_sub_self->___reaction_0.num_outputs);
    }
    // Initialize the output_produced array.
    // Total number of outputs produced by the reaction.
    spatreceiver_sub_self->___reaction_1.num_outputs = 1;
    // Allocate arrays for triggering downstream reactions.
    if (spatreceiver_sub_self->___reaction_1.num_outputs > 0) {
        spatreceiver_sub_self->___reaction_1.output_produced = (bool**)malloc(sizeof(bool*) * spatreceiver_sub_self->___reaction_1.num_outputs);
        spatreceiver_sub_self->___reaction_1.triggers = (trigger_t***)malloc(sizeof(trigger_t**) * spatreceiver_sub_self->___reaction_1.num_outputs);
        spatreceiver_sub_self->___reaction_1.triggered_sizes = (int*)malloc(sizeof(int) * spatreceiver_sub_self->___reaction_1.num_outputs);
    }
    // Initialize the output_produced array.
    spatreceiver_sub_self->___reaction_1.output_produced[0]
    = &spatreceiver_sub_self->__message.is_present
    ;
    // Total number of outputs produced by the reaction.
    spatreceiver_sub_self->___reaction_2.num_outputs = 0;
    // Allocate arrays for triggering downstream reactions.
    if (spatreceiver_sub_self->___reaction_2.num_outputs > 0) {
        spatreceiver_sub_self->___reaction_2.output_produced = (bool**)malloc(sizeof(bool*) * spatreceiver_sub_self->___reaction_2.num_outputs);
        spatreceiver_sub_self->___reaction_2.triggers = (trigger_t***)malloc(sizeof(trigger_t**) * spatreceiver_sub_self->___reaction_2.num_outputs);
        spatreceiver_sub_self->___reaction_2.triggered_sizes = (int*)malloc(sizeof(int) * spatreceiver_sub_self->___reaction_2.num_outputs);
    }
    // Initialize the output_produced array.
    // Reaction 0 of spatReceiver.sub does not depend on one maximal upstream reaction.
    spatreceiver_sub_self->___reaction_0.last_enabling_reaction = NULL;
    // Reaction 1 of spatReceiver.sub depends on one maximal upstream reaction.
    spatreceiver_sub_self->___reaction_1.last_enabling_reaction = &(spatreceiver_sub_self->___reaction_0);
    // Reaction 1 of spatReceiver.sub triggers 1 downstream reactions through port spatReceiver.sub.message.
    spatreceiver_sub_self->___reaction_1.triggered_sizes[0] = 1;
    // For reaction 1 of spatReceiver.sub, allocate an
    // array of trigger pointers for downstream reactions through port spatReceiver.sub.message
    trigger_t** spatreceiver_sub_1_0 = (trigger_t**)malloc(1 * sizeof(trigger_t*));
    spatreceiver_sub_self->___reaction_1.triggers[0] = spatreceiver_sub_1_0;
    // Point to destination port spatReceiver.rsuMsg.message's trigger struct.
    spatreceiver_sub_1_0[0] = &spatreceiver_rsumsg_self->___message;
    // Reaction 2 of spatReceiver.sub depends on one maximal upstream reaction.
    spatreceiver_sub_self->___reaction_2.last_enabling_reaction = &(spatreceiver_sub_self->___reaction_1);
    spatreceiver_sub_self->__message.num_destinations
    = 1;
    // doDeferredInitialize
    spatreceiver_sub_self->__message.token = __create_token(sizeof(char));
    // Connect inputs and outputs for reactor spatReceiver.
    // Connect spatReceiver.sub.message to input port spatReceiver.rsuMsg.message
    spatreceiver_rsumsg_self->__message = (recvspatmessage2_message_t*)&spatreceiver_sub_self->__message;
    // Connect inputs and outputs for reactor spatReceiver.rsuMsg.
    // END Connect inputs and outputs for reactor spatReceiver.rsuMsg.
    // Connect inputs and outputs for reactor spatReceiver.sub.
    // END Connect inputs and outputs for reactor spatReceiver.sub.
    // END Connect inputs and outputs for reactor spatReceiver.
    // Add action spatReceiver.sub.incoming_message to array of is_present fields.
    __is_present_fields[0] 
            = &spatreceiver_sub_self->__incoming_message.is_present;
    __tokens_with_ref_count[1].token
            = &spatreceiver_rsumsg_self->__message->token;
    __tokens_with_ref_count[1].is_present
            = &spatreceiver_rsumsg_self->__message->is_present;
    __tokens_with_ref_count[1].reset_is_present = false;
    // Add port spatReceiver.sub.message to array of is_present fields.
    __is_present_fields[1] = &spatreceiver_sub_self->__message.is_present
    ;
    spatreceiver_rsumsg_self->___reaction_0.chain_id = 1;
    // index is the OR of level 2 and 
    // deadline 140737488355327 shifted left 16 bits.
    spatreceiver_rsumsg_self->___reaction_0.index = 0x7fffffffffff0002LL;
    spatreceiver_sub_self->___reaction_0.chain_id = 3;
    // index is the OR of level 0 and 
    // deadline 140737488355327 shifted left 16 bits.
    spatreceiver_sub_self->___reaction_0.index = 0x7fffffffffff0000LL;
    spatreceiver_sub_self->___reaction_1.chain_id = 3;
    // index is the OR of level 1 and 
    // deadline 140737488355327 shifted left 16 bits.
    spatreceiver_sub_self->___reaction_1.index = 0x7fffffffffff0001LL;
    spatreceiver_sub_self->___reaction_2.chain_id = 2;
    // index is the OR of level 2 and 
    // deadline 140737488355327 shifted left 16 bits.
    spatreceiver_sub_self->___reaction_2.index = 0x7fffffffffff0002LL;
}
void __trigger_startup_reactions() {
    if (number_of_threads == 0) {
       number_of_threads = 1;
    }
    for (int i = 0; i < __startup_reactions_size; i++) {
        if (__startup_reactions[i] != NULL) {
            _lf_enqueue_reaction(__startup_reactions[i]);
        }
    }
}
void __initialize_timers() {
}
void logical_time_complete(instant_t time) {
}
instant_t next_event_time(instant_t time) {
    return time;
}
bool __wrapup() {
    __start_time_step();  // To free memory allocated for actions.
    for (int i = 0; i < __shutdown_triggers_size; i++) {
        __schedule(__shutdown_triggers[i], 0LL, NULL);
    }
    // Return true if there are shutdown actions.
    return (__shutdown_triggers_size > 0);
}
void __termination() {}