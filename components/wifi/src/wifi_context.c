// wifi_context.c

#include "wifi_internals.h"

// Global WiFi context
wifi_context_t g_wifi_ctx = {
    .state = WIFI_DISCONNECTED,
    .ssid = "",
    .rssi = 0,
    .last_disconnect_reason = 0,
    .netif = NULL,
    .ip = {
        .has_ip = false,
        .ip_str = "",
    },
    .ip6 = {
        .has_ip = false,
        .ip_str = "",
    }};

esp_err_t wifi_context_init(void)
{
    memset(&g_wifi_ctx, 0, sizeof(wifi_context_t));
    g_wifi_ctx.state = WIFI_DISCONNECTED;
    return ESP_OK;
}

const wifi_context_t *get_wifi_context(void)
{
    return &g_wifi_ctx;
}

void wifi_context_update_state(wifi_state_t new_state)
{
    if (new_state != g_wifi_ctx.state)
    {
        ESP_LOGI(WIFI_CTX_TAG, "WiFi state changed: %d -> %d", g_wifi_ctx.state, new_state);
        g_wifi_ctx.state = new_state;

        // Clear IP information on disconnect
        if (new_state == WIFI_DISCONNECTED)
        {
            g_wifi_ctx.ip.has_ip = false;
            g_wifi_ctx.ip.ip_str[0] = '\0';
            g_wifi_ctx.ip6.has_ip = false;
            g_wifi_ctx.ip6.ip_str[0] = '\0';
        }
    }
}

void wifi_context_update_connection(const char *ssid, int8_t rssi)
{
    if (ssid)
    {
        strncpy(g_wifi_ctx.ssid, ssid, sizeof(g_wifi_ctx.ssid) - 1);
        g_wifi_ctx.ssid[sizeof(g_wifi_ctx.ssid) - 1] = '\0';
    }
    g_wifi_ctx.rssi = rssi;
}

void wifi_context_update_disconnect_reason(uint8_t reason)
{
    g_wifi_ctx.last_disconnect_reason = reason;
}

void wifi_context_update_ip(const esp_netif_ip_info_t *ip_info)
{
    if (!ip_info)
        return;

    g_wifi_ctx.ip.ip = ip_info->ip;
    g_wifi_ctx.ip.netmask = ip_info->netmask;
    g_wifi_ctx.ip.gateway = ip_info->gw;
    g_wifi_ctx.ip.has_ip = true;

    // Convert IP to string representation
    snprintf(g_wifi_ctx.ip.ip_str, sizeof(g_wifi_ctx.ip.ip_str),
             IPSTR, IP2STR(&ip_info->ip));

    ESP_LOGI(WIFI_CTX_TAG, "IP Updated: %s", g_wifi_ctx.ip.ip_str);
}

void wifi_context_update_ip6(const esp_ip6_addr_t *ip6_addr)
{
    if (!ip6_addr)
        return;

    memcpy(&g_wifi_ctx.ip6.ip, ip6_addr, sizeof(esp_ip6_addr_t));
    g_wifi_ctx.ip6.has_ip = true;

    // Convert IPv6 to string representation
    snprintf(g_wifi_ctx.ip6.ip_str, sizeof(g_wifi_ctx.ip6.ip_str),
             IPV6STR, IPV62STR(*ip6_addr));

    ESP_LOGI(WIFI_CTX_TAG, "IPv6 Updated: %s", g_wifi_ctx.ip6.ip_str);
}