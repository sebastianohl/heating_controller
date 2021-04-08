#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <string.h>

#include "remote_log.h"

#define TAG "REMOTE_LOG"

/* log level */
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_INFO 6
#define LOG_DEBUG 7

/* facility codes */
#define LOG_USER 1
#define LOG_DAEMON 3

struct sockaddr_in destAddr;
int sock = 0;
struct sockaddr_in destAddrSyslog;
int sockSyslog = 0;
char hostnameSyslog[255] = {0};
char log_buffer[256] = {0};

#ifdef CONFIG_LOG_COLORS
#error "please disable color codes for logging in sdk config"
#endif

void logSyslog(char* log)
{
    char appname[255]= {0};
    uint8_t pri = LOG_DAEMON*8;
    char level;
    uint32_t time;
    uint32_t last_match = 0;
    
    if (sscanf(log, "%c (%d) %s %n", &level, &time, appname, &last_match) > 0)
    {
        appname[strlen(appname)] = 0; /* remove colon */
        switch (level)
        {
            case 'E': pri += LOG_ERR; break;
            case 'W': pri += LOG_WARNING; break;
            case 'I': pri += LOG_INFO; break;
            case 'D':
            case 'V': 
            default: pri += LOG_DEBUG; break;
        }
    } else {
        pri += LOG_ERR;
    }

    char* packet = malloc(20+strlen(hostnameSyslog)+strlen(appname)+strlen(log+last_match)+1);

    // IETF Doc: https://tools.ietf.org/html/rfc5424
    sprintf(packet, "<%d>1 - %s %s - - - \xEF\xBB\xBF%s", pri, hostnameSyslog, appname, log+last_match); // BOM UTF-8
    sendto(sockSyslog, packet, strlen(packet), 0, (struct sockaddr *)&destAddrSyslog, sizeof(destAddrSyslog));
    free(packet);
}
#ifdef ESP_PLATFORM // esp32
int (*old_log)(const char *, va_list) = NULL;
int remote_log_vprintf(const char *fmt, va_list args)
{
    vsnprintf(log_buffer, sizeof(log_buffer)-1, fmt, args);
    sendto(sock, log_buffer, strlen(log_buffer), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
    logSyslog(log_buffer);
    return old_log(fmt, args);
}
#else // esp8266
int (*old_log) (int) = NULL;
int remote_log_putchar(int c)
{
    int len = strlen(log_buffer);
    if (len+1 >= sizeof(log_buffer)) // buffer full, send log and clear log
    {
        sendto(sock, log_buffer, strlen(log_buffer), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
        logSyslog(log_buffer);
        log_buffer[0] = 0;
        len = 0;
    }
    log_buffer[len] = c;
    log_buffer[len+1] = 0;
    if (c == '\n') // send each line
    {
        sendto(sock, log_buffer, strlen(log_buffer), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
        log_buffer[len] = 0; // remove CR
        logSyslog(log_buffer);
        log_buffer[0] = 0;
    }

    return old_log(c); // log to serial
}
#endif
void start_remote_log(const char *ip_addr, const int port, const char *syslog_ip_addr, const int syslog_port, const char* syslog_hostname)
{
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    char addr_str[128];

    if (old_log != NULL)
    {
        return;
    }
    destAddr.sin_addr.s_addr = inet_addr(ip_addr);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    destAddrSyslog.sin_addr.s_addr = inet_addr(syslog_ip_addr);
    destAddrSyslog.sin_family = AF_INET;
    destAddrSyslog.sin_port = htons(syslog_port);
    inet_ntoa_r(destAddrSyslog.sin_addr, addr_str, sizeof(addr_str) - 1);

    sockSyslog = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sockSyslog < 0) {
        ESP_LOGE(TAG, "Unable to create syslog socket: errno %d", errno);
        return;
    }

    snprintf(hostnameSyslog, sizeof(hostnameSyslog), "%s", syslog_hostname);

    ESP_LOGI(TAG, "enable remote log");
    memset(log_buffer, 0, sizeof(log_buffer));
    #ifdef ESP_PLATFORM // esp 32
    old_log = esp_log_set_vprintf(&remote_log_vprintf);
    #else // esp8266
    old_log = esp_log_set_putchar(&remote_log_putchar);
    #endif
    ESP_LOGI(TAG, "remote log enabled");
}

void stop_remote_log()
{
    #ifdef ESP_PLATFORM // esp 32
    old_log = esp_log_set_vprintf(old_log);
    #else // esp8266
    old_log = esp_log_set_putchar(old_log);
    #endif
    close(sock);
    close(sockSyslog);
    sock = 0;
    sockSyslog = 0;
    ESP_LOGI(TAG, "remote log disabled");
}

