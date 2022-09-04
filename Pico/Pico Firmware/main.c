#include <string.h>
#include <time.h>

#include "hardware/structs/rosc.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp_tcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "hardware/pio.h"
#include "pio/pio_spi.h"

#include "ws.h"
#include "multiboot.h"
#include "sio.h"

#define TW_HOSTNAME         "tileworld.org"
#define TW_PORT             7364

#define GH_HOSTNAME        "raw.githubusercontent.com"
#define GH_PORT            443

#define TLS_CLIENT_WWS_UPGRADE_REQUEST  "GET / HTTP/1.1\r\n" \
                                        "Host: tileworld.org:7364\r\n" \
                                        "Origin: https://tileworld.org:7364\r\n" \
                                        "Upgrade: websocket\r\n" \
                                        "Connection: Upgrade\r\n" \
                                        "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n" \
                                        "Sec-WebSocket-Version: 13\r\n\r\n"

#define TLS_CLIENT_GH_ROM_GET_REQUEST   "GET https://raw.githubusercontent.com/Squaresweets/TileWorldGBA/main/TileWorldGBA_mb.gba HTTP/1.1\r\n" \
                                        "Host: raw.githubusercontent.com\r\n\r\n"

#define TLS_CLIENT_TIMEOUT_SECS  15
#define BUF_SIZE               57604

typedef struct {
    struct altcp_pcb *pcb;
    bool complete;
    uint8_t  buf[BUF_SIZE]; //For data from Tileworld->GBA
    uint64_t buffer_pos;
    uint64_t buffer_len;

    u16_t port;
    uint32_t recvNum;
} TLS_CLIENT_T;

uint8_t  outBuf[20]; //For data from GBA->TileWorld
uint64_t out_buffer_pos;
uint64_t out_buffer_len;

static struct altcp_tls_config *tls_config = NULL;

/*------------- PIO -------------*/
  pio_spi_inst_t spi = {
          .pio = pio1,
          .sm = 0
  };


#define TileWorldThingamabobLayout

#ifdef TileWorldThingamabobLayout
#define PIN_SCK 21
#define PIN_SIN 20
#define PIN_SOUT 19
#else
#define PIN_SCK 0
#define PIN_SIN 1
#define PIN_SOUT 2
#endif


#pragma region tlsCallbacks
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

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    if (err != ERR_OK) {
        printf("connect failed %d\n", err);
        return tls_client_close(state);
    }

    printf("connected to server\n");
    if(state->port == GH_PORT) //If we are connecting to github send the request to download the ROM
        err = altcp_write(state->pcb, TLS_CLIENT_GH_ROM_GET_REQUEST, strlen(TLS_CLIENT_GH_ROM_GET_REQUEST), TCP_WRITE_FLAG_COPY);
    else if(state->port == TW_PORT) //If we are connecting to tileworld send a WS upgrade request
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
#pragma endregion

static err_t ws_client_send(TLS_CLIENT_T *state, void *dataptr, uint64_t len)
{
    printf("\n*****************SENDING*****************\n");
    for(int i = 0; i<len; i++)
        printf("%#x ", ((char *) dataptr)[i]);
    printf("\n");
    
    char buffer[32];
    uint64_t numInBuf = WSBuildPacket(buffer, 32, WEBSOCKET_OPCODE_BIN, dataptr, len);
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

//This function handles sending data from TileWorld->GBA
//But more importantly keeps track of packet lengths and the outbuf for GBA->TileWorld
void handleSIO(uint32_t data, TLS_CLIENT_T *state)
{
    //Data is what we are sending
    //This is entirelly optional, and if there is currently no data being recieved from the server
    //This will just be 0, this is fine as that is sorted out by the GBA
    uint32_t rx = rw4(data); //send and recieve all data!

    //Now we start doing GBA->TileWorld stuff
    if(out_buffer_len == 0)
    {
        out_buffer_len = data;
        out_buffer_pos = 0;
        return;
    }

    outBuf[out_buffer_pos++] = ((data >> 24) & 0xFF);
    outBuf[out_buffer_pos++] = ((data >> 16) & 0xFF);
    outBuf[out_buffer_pos++] = ((data >> 8) & 0xFF);
    outBuf[out_buffer_pos++] = (data & 0xFF);

    if(out_buffer_pos >= out_buffer_len) //We have recieved a full message, time to send it on
    {
        ws_client_send(state, outBuf, out_buffer_len);
        out_buffer_len = 0;
    }
}

static err_t ws_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p, err_t err) {
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;
    state->recvNum++;

    if (!p) {
        printf("connection closed\n");
        return tls_client_close(state);
    }

    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        char msgBuf[p->tot_len + 1];
        pbuf_copy_partial(p, msgBuf, p->tot_len, 0);
        msgBuf[p->tot_len] = 0; //Not sure why it does this in the example but who am I to complain

#pragma region github
        //************************************ CONNECTING TO GITHUB ************************************
        if(state->port == GH_PORT)
        {
            if(state->recvNum == 1) //This is the first message we recieved and so will be a "HTTP/1.1 200 OK" message
            {
                /*the +57 and strtok removes:
                HTTP/1.1 200 OK
                Connection: keep-alive
                Content-Length: 
                And then everything after the number
                Then we convert it to an int*/
                char* result = msgBuf + 57;
                result = strtok(result, "\n");
                int len = strtol(result, NULL, 10);
                printf("\nLength of ROM:%d\n", len);
                state->buffer_len = len;
            }
            else
            {
                memcpy(&state->buf[state->buffer_pos], msgBuf, p->tot_len);
                state->buffer_pos += p->tot_len;
                if(state->buffer_pos == state->buffer_len) //We have recieved the entire game rom!
                {
                    printf("Game ROM recieved! Starting multiboot!\n");
                    multiboot(state->buf, state->buffer_len);
                    state->complete = true; //Disconnect from GH then connect to TW
                }
            }
        }
#pragma endregion
        //************************************ CONNECTING TO TILEWORLD ************************************
        else if(state->port == TW_PORT)
        {
            uint64_t lenReceived = (uint64_t)p->tot_len;
            if(state->buffer_len == 0)  //This is a new message!
            {
                WebsocketPacketHeader_t header;
                WSParsePacket(&header, msgBuf, p->tot_len); //Parse the packet only when it is a new message
                printf("\n\n\nStart of a new message! Buffer size: %llu\n", header.totalLen);
                
                state->buffer_len = header.totalLen; 
                lenReceived = header.payloadLen;
            }
            printf("Reciveved %llu bytes!\n", lenReceived);
            memcpy(&state->buf[state->buffer_pos], msgBuf, lenReceived);
            state->buffer_pos += lenReceived; //Add however many bytes we recieved (either tot_len or header.payloadlen)

            if(state->buffer_pos >= state->buffer_len) //Recieved all bytes
            {
                for(int i = 0; i<(state->buffer_len/4)+1; i++) //loop through each u32 in the message
                    handleSIO(((u32_t*)outBuf)[i], state);
                state->buffer_len = 0; //Have to decrypt the header for the next one
                state->buffer_pos = 0;
            }
        }
        altcp_recved(pcb, p->tot_len);

    }
    pbuf_free(p);

    /*
    if(!testflag)
    {
        uint8_t test[3] = {0x01, 0x01, 0x00};
        ws_client_send(state, test, 3);
        testflag = true;
    }
    */
    return ERR_OK;
}
#pragma region TLSConnect
static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr, TLS_CLIENT_T *state)
{
    err_t err;

    printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), state->port);
    err = altcp_connect(state->pcb, ipaddr, state->port, tls_client_connected);
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


static bool tls_client_open(const char *hostname, void *arg, u16_t port) {
    err_t err;
    ip_addr_t server_ip;
    TLS_CLIENT_T *state = (TLS_CLIENT_T*)arg;

    state->port = port;
    memset(state->buf, 0, BUF_SIZE); //Default all values to 0 (only used for multiboot stuff)

    state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
    if (!state->pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    altcp_arg(state->pcb, state);
    altcp_poll(state->pcb, tls_client_poll, TLS_CLIENT_TIMEOUT_SECS * 2);
    altcp_recv(state->pcb, ws_client_recv);
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
    if (err != ERR_INPROGRESS)
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
#pragma endregion
void run_TLS_CLIENT(char* hostname, u16_t port) {
    /* No CA certificate checking */
    tls_config = altcp_tls_create_config_client(NULL, 0);

    TLS_CLIENT_T *state = tls_client_init();
    if (!state) {
        return;
    }
    if (!tls_client_open(hostname, state, port)) {
        return;
    }
    while(!state->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for WiFi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        handleSIO(0, state);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
#endif
    }
    free(state);
    altcp_tls_free_config(tls_config);
}

int main() {
    stdio_init_all();
    
    uint cpha1_prog_offs = pio_add_program(spi.pio, &spi_cpha1_program);
    pio_spi_init(spi.pio, spi.sm, cpha1_prog_offs, 8, 32, 1, 1, PIN_SCK, PIN_SOUT, PIN_SIN);
    ////129.8828125 519.53125
    sio_spi = &spi; //In sio.h

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
    
    /*Throughout this program we make two connections, one to connect to github and download
    the rom and one to actually connect to tileworld*/
    run_TLS_CLIENT(GH_HOSTNAME, GH_PORT); //First connect to github
    run_TLS_CLIENT(TW_HOSTNAME, TW_PORT); //Then connect to TW

    /* sleep a bit to let usb stdio write out any buffer to host */
    sleep_ms(100);

    cyw43_arch_deinit();
    return 0;
}

