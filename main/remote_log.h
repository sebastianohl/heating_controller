#ifndef REMOTE_LOG_H_
#define REMOTE_LOG_H_

void start_remote_log(const char *ip_addr, const int port,
                      const char *syslog_ip_addr, const int syslog_port,
                      const char *syslog_hostname);
void stop_remote_log();

#endif