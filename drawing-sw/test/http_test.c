#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8080

// Callback to handle HTTP requests
static int handle_request(void *cls, struct MHD_Connection *connection, 
                          const char *url, const char *method, 
                          const char *version, const char *upload_data, 
                          size_t *upload_data_size, void **con_cls) {
    static int dummy;
    const char *response_str = "Message received!";
    struct MHD_Response *response;

    // Only handle POST requests
    if (strcmp(method, "POST") != 0) {
        return MHD_NO; // Reject non-POST requests
    }

    if (*con_cls == NULL) {
        *con_cls = &dummy;
        return MHD_YES;
    }

    if (*upload_data_size > 0) {
        printf("Received message: %s\n", upload_data);
        *upload_data_size = 0; // Signal that we've processed this data
        return MHD_YES;
    }

    // Send response
    response = MHD_create_response_from_buffer(strlen(response_str), 
                                               (void *)response_str, 
                                               MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    // Start the HTTP server
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, 
                              &handle_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start HTTP server\n");
        return 1;
    }

    printf("HTTP server running on port %d\n", PORT);

    // Run server indefinitely
    getchar();

    // Stop the server
    MHD_stop_daemon(daemon);

    return 0;
}
