#include <string.h>
#include <time.h>

#include "hardware/structs/rosc.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "ws.h"

#define TLS_CLIENT_SERVER        "tileworld.org"
#define PORT_NUMBER              7364

#define TLS_CLIENT_WWS_UPGRADE_REQUEST  "GET / HTTP/1.1\r\n" \
                                        "Host: tileworld.org:7364\r\n" \
                                        "Origin: https://tileworld.org:7364\r\n" \
                                        "Upgrade: websocket\r\n" \
                                        "Connection: Upgrade\r\n" \
                                        "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n" \
                                        "Sec-WebSocket-Version: 13\r\n\r\n"
#define TLS_CLIENT_TIMEOUT_SECS  15
#define BUF_SIZE               32
#define rx_BUF_SIZE            57601

typedef struct {
    struct altcp_pcb *pcb;
    bool complete;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

bool testflag = false;

/* Function to feed mbedtls entropy. May be better to move it to pico-sdk */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
    /* Code borrowed from pico_lwip_random_byte(), which is static, so we cannot call it directly */
    static uint8_t byte;

    for(int p=0; p<len; p++) {
        for(int i=0;i<32;i++) {
            // picked a fairly arbitrary polynomial of 0x35u - this doesn't have to be crazily uniform.
            byte = ((byte << 1) | rosc_hw->randombit) ^ (byte & 0x80u ? 0x35u : 0);
            // delay a little because the random bit is a little slow
            busy_wait_at_least_cycles(30);
        }
        output[p] = byte;
    }

    *olen = len;
    return 0;
}


static err_t tls_client_close(void *arg) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    err_t err = ERR_OK;

    state->complete = true;
    if (state->pcb != NULL) {
        altcp_arg(state->pcb, NULL);
        altcp_poll(state->pcb, NULL, 0);
        altcp_recv(state->pcb, NULL);
        altcp_err(state->pcb, NULL);
        err = altcp_close(state->pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            altcp_abort(state->pcb);
            err = ERR_ABRT;
        }
        state->pcb = NULL;
    }
    return err;
}

static err_t ws_client_send(TLS_CLIENT_T *state, void *dataptr, uint64_t len)
{
    printf("\n*****************SENDING*****************\n");
    for(int i = 0; i<len; i++)
        printf("%#x ", ((char *) dataptr)[i]);
    printf("\n");
    
    char buffer[BUF_SIZE];
    uint64_t numInBuf = WSBuildPacket(buffer, BUF_SIZE, WEBSOCKET_OPCODE_BIN, dataptr, len);
    printf("Actual data we are sending:\n");
    for(uint64_t i = 0; i<numInBuf; i++)
        printf("%#X ", buffer[i]);
    printf("\n\n\n\n");

    err_t err = altcp_write(state->pcb, buffer, numInBuf, TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }

    return ERR_OK;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server, sending upgrade request\n");
    err = altcp_write(state->pcb, TLS_CLIENT_WWS_UPGRADE_REQUEST, strlen(TLS_CLIENT_WWS_UPGRADE_REQUEST), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("error writing data, err=%d", err);
        return tls_client_close(state);
    }


    return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
    printf("timed out");
    return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    printf("tls_client_err %d\n", err);
    state->pcb = NULL; /* pcb freed by lwip when _err function is called */
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    if (p->tot_len > 0) {
        /* For simplicity this examples creates a buffer on stack the size of the data pending here, 
           and copies all the data to it in one go.
           Do be aware that the amount of data can potentially be a bit large (TLS record size can be 16 KB),
           so you may want to use a smaller fixed size buffer and copy the data to it using a loop, if memory is a concern */
        printf("\n*************************RECIEVING************************* %d\n", p->tot_len);
        char buf[p->tot_len + 1];

        pbuf_copy_partial(p, buf, p->tot_len, 0);
        buf[p->tot_len] = 0;
        printf("%s\n", buf); //Print text data

        WebsocketPacketHeader_t header;
        WSParsePacket(&header, buf, p->tot_len);

        printf("Unencrypted data: \n");
        for(int i = 0; i<header.length; i++)
            printf("%#X ", buf[i]);
        printf("\n\n\n\n");

        altcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    if(!testflag)
    {
        //uint8_t test[12] = {0x09, 0x00, 0x09, 0x70, 0x69, 0x63, 0x6f, 0x20, 0x74, 0x65, 0x73, 0x74};
        uint8_t test[3] = {0x01, 0x01, 0x00};
        ws_client_send(state, test, 3);
        testflag = true;
    }
    else
    {
        uint8_t test[12] = {6, 0, 0, 0, 18, 0, 0, 0, 8, 1, 8, 0};
        //ws_client_send(state, test, 12);
    }

    return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), PORT_NUMBER);
    err = altcp_connect(state->pcb, ipaddr, PORT_NUMBER, tls_client_connected);
    if (err != ERR_OK)
    {
        fprintf(stderr, "error initiating connect, err=%d\n", err);
        tls_client_close(state);
    }
}

static void tls_client_dns_found(const char* hostname, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr)
    {
        printf("DNS resolving complete\n");
        tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *) arg);
    }
    else
    {
        printf("error resolving hostname %s\n", hostname);
        tls_client_close(arg);
    }
}


static bool tls_client_open(const char *hostname, void *arg) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;

    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, TLS_CLIENT_TIMEOUT_SECS * 2);
    altcp_recv(state->pcb, tls_client_recv);
    altcp_err(state->pcb, tls_client_err);

    /* Set SNI */
    mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);

    printf("resolving %s\n", hostname);

    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();

    err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
    if (err == ERR_OK)
    {
        /* host is in DNS cache */
        tls_client_connect_to_server_ip(&server_ip, state);
    }
    else if (err != ERR_INPROGRESS)
    {
        printf("error initiating DNS resolving, err=%d\n", err);
        tls_client_close(state->pcb);
    }

    cyw43_arch_lwip_end();

    return err == ERR_OK || err == ERR_INPROGRESS;
}

// Perform initialisation
static TLS_CLIENT_T* tls_client_init(void) {
    TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
    if (!state) {
        printf("failed to allocate state\n");
        return NULL;
    }

    return state;
}

void run_TLS_CLIENT(void) {
    /* No CA certificate checking */
    tls_config = altcp_tls_create_config_client(NULL, 0);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        return;
    }
    if (!tls_client_open(TLS_CLIENT_SERVER, state)) {
        return;
    }
    while(!state->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for WiFi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        sleep_ms(1);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    free(state);
    altcp_tls_free_config(tls_config);
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    //if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    while (cyw43_arch_wifi_connect_timeout_ms("NETGEAR29", "pinkflute287", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect to wifi, trying again!\n");
    }
    printf("Connected to wifi!\n");
    
    run_TLS_CLIENT();

    /* sleep a bit to let usb stdio write out any buffer to host */
    sleep_ms(100);

    cyw43_arch_deinit();
    return 0;
}

