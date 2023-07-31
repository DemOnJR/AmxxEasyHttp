#include "utils/ftp_utils.h"
#include "utils/string_utils.h"
#include "EasyHttpModule.h"
#include <algorithm>
#include <utility>

#include <amxxmodule.h>
#include <undef_metamod.h>

using namespace ezhttp;

bool ValidateOptionsId(AMX* amx, OptionsId options_id);
bool ValidateRequestId(AMX* amx, RequestId request_id);
bool ValidateQueueId(AMX* amx, QueueId queue_id);
template <class TMethod> void SetKeyValueOption(AMX* amx, cell* params, TMethod method);
template <class TMethod> void SetStringOption(AMX* amx, cell* params, TMethod method);
RequestId SendRequest(AMX* amx, RequestMethod method, OptionsId options_id, const std::string& url, const std::string& callback);

std::unique_ptr<EasyHttpModule> g_EasyHttpModule;

cell AMX_NATIVE_CALL ezhttp_create_options(AMX* amx, cell* params)
{
    OptionsId options_id = g_EasyHttpModule->CreateOptions();

    return (int)options_id;
}

cell AMX_NATIVE_CALL ezhttp_destroy_options(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    return g_EasyHttpModule->DeleteOptions(options_id);
}

cell AMX_NATIVE_CALL ezhttp_option_set_user_agent(AMX* amx, cell* params)
{
    SetStringOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetUserAgent);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_add_url_parameter(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::AddUrlParameter);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_add_form_payload(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::AddFormPayload);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_body(AMX* amx, cell* params)
{
    SetStringOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetBody);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_append_body(AMX* amx, cell* params)
{
    SetStringOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::AppendBody);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_header(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetHeader);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_cookie(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetCookie);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_timeout(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];
    cell timeout_ms = params[2];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    g_EasyHttpModule->GetOptions(options_id).options_builder.SetTimeout(timeout_ms);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_connect_timeout(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];
    cell timeout_ms = params[2];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    g_EasyHttpModule->GetOptions(options_id).options_builder.SetConnectTimeout(timeout_ms);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_proxy(AMX* amx, cell* params)
{
    SetStringOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetProxy);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_proxy_auth(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetProxyAuth);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_auth(AMX* amx, cell* params)
{
    SetKeyValueOption(amx, params, &ezhttp::EasyHttpOptionsBuilder::SetAuth);
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_user_data(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];
    cell* data_addr = MF_GetAmxAddr(amx, params[2]);
    int data_len = params[3];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    std::vector<cell> user_data;
    user_data.resize(data_len);
    MF_CopyAmxMemory(user_data.data(), data_addr, data_len);

    g_EasyHttpModule->GetOptions(options_id).user_data = user_data;
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_plugin_end_behaviour(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];
    auto plugin_end_behaviour = (PluginEndBehaviour)params[2];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    g_EasyHttpModule->GetOptions(options_id).plugin_end_behaviour = plugin_end_behaviour;
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_option_set_queue(AMX* amx, cell* params)
{
    auto options_id = (OptionsId)params[1];
    auto queue_id = (QueueId)params[2];

    if (!ValidateOptionsId(amx, options_id))
        return 0;

    if (!ValidateQueueId(amx, queue_id))
        return 0;

    g_EasyHttpModule->GetOptions(options_id).queue_id = queue_id;
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get(AMX* amx, cell* params)
{
    int url_len;
    char* url = MF_GetAmxString(amx, params[1], 0, &url_len);

    int callback_len;
    char* callback = MF_GetAmxString(amx, params[2], 1, &callback_len);

    auto options_id = (OptionsId)params[3];

    return (cell)SendRequest(amx, RequestMethod::HttpGet, options_id, std::string(url, url_len), std::string(callback, callback_len));
}

cell AMX_NATIVE_CALL ezhttp_post(AMX* amx, cell* params)
{
    int url_len;
    char* url = MF_GetAmxString(amx, params[1], 0, &url_len);

    int callback_len;
    char* callback = MF_GetAmxString(amx, params[2], 1, &callback_len);

    auto options_id = (OptionsId)params[3];

    return (cell)SendRequest(amx, RequestMethod::HttpPost, options_id, std::string(url, url_len), std::string(callback, callback_len));
}

cell AMX_NATIVE_CALL ezhttp_is_request_exists(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    return g_EasyHttpModule->IsRequestExists(request_id);
}

cell AMX_NATIVE_CALL ezhttp_cancel_request(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    RequestData& request_data = g_EasyHttpModule->GetRequest(request_id);

    std::lock_guard<std::mutex> lock_guard(request_data.request_control->control_mutex);
    request_data.request_control->canceled = true;

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_request_progress(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    auto& request_control = g_EasyHttpModule->GetRequest(request_id).request_control;
    std::lock_guard<std::mutex> lock_guard(request_control->control_mutex);

    cell* p = MF_GetAmxAddr(amx, params[2]);
    p[0] = request_control->progress.download_now;
    p[1] = request_control->progress.download_total;
    p[2] = request_control->progress.upload_now;
    p[3] = request_control->progress.upload_total;

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get_http_code(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return response.status_code;
}

cell AMX_NATIVE_CALL ezhttp_get_data(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    cell max_len = params[3];

    if (!ValidateRequestId(amx, request_id) || max_len == 0)
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    MF_SetAmxString(amx, params[2], response.text.c_str(), max_len);

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get_url(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    cell max_len = params[3];

    if (!ValidateRequestId(amx, request_id) || max_len == 0)
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    MF_SetAmxString(amx, params[2], response.url.c_str(), max_len);

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_save_data_to_file(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    int file_path_len;
    char* file_path = MF_GetAmxString(amx, params[2], 0, &file_path_len);
    cell max_len = params[3];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    if (response.text.empty())
        return 0;

    std::ofstream file(MF_BuildPathname("%s", file_path), std::ofstream::out | std::ofstream::binary);
    if (!file.is_open())
        return 0;

    file.write(response.text.data(), response.text.length());
    file.close();

    return response.text.length();
}

cell AMX_NATIVE_CALL ezhttp_save_data_to_file2(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    FILE* file_handle = (FILE*)params[2];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return std::fwrite(response.text.data(), sizeof(char), response.text.length(), file_handle);
}

cell AMX_NATIVE_CALL ezhttp_get_headers_count(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return response.header.size();
}

cell AMX_NATIVE_CALL ezhttp_get_headers(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    int key_len;
    char* key = MF_GetAmxString(amx, params[2], 0, &key_len);
    cell value_max_len = params[4];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    const std::string header_key(key, key_len);
    if (response.header.count(header_key) == 1)
    {
        const std::string& header_value = response.header.at(header_key);

        MF_SetAmxString(amx, params[3], header_value.c_str(), value_max_len);

        return 1;
    }

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_iterate_headers(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    cell iter = params[2];
    int key_len;
    char* key = MF_GetAmxString(amx, params[3], 0, &key_len);
    cell value_max_len = params[5];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const RequestData& request = g_EasyHttpModule->GetRequest(request_id);
    const cpr::Header& header = request.response.header;


    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get_elapsed(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return amx_ftoc(response.elapsed);
}

cell AMX_NATIVE_CALL ezhttp_get_cookies_count(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    size_t size = response.cookies.end() - response.cookies.begin();
    return (cell)size;
}

cell AMX_NATIVE_CALL ezhttp_get_cookies(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    int key_len;
    char* key = MF_GetAmxString(amx, params[2], 0, &key_len);
    cell value_max_len = params[4];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    const std::string cookie_key(key, key_len);

    auto it = std::find_if(response.cookies.begin(), response.cookies.end(), [&cookie_key](const auto& item) { return item.GetName() == cookie_key; });
    if (it != response.cookies.end())
    {
        const std::string& cookie_value = it->GetValue();

        MF_SetAmxString(amx, params[3], cookie_value.c_str(), value_max_len);

        return 1;
    }

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_iterate_cookies(AMX* amx, cell* params)
{
    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get_error_code(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return (cell)response.error.code;
}

cell AMX_NATIVE_CALL ezhttp_get_error_message(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    cell max_len = params[3];

    if (!ValidateRequestId(amx, request_id) || max_len == 0)
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    MF_SetAmxString(amx, params[2], response.error.message.c_str(), max_len);

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_get_redirect_count(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return response.redirect_count;
}

cell AMX_NATIVE_CALL ezhttp_get_uploaded_bytes(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return response.uploaded_bytes;
}

cell AMX_NATIVE_CALL ezhttp_get_downloaded_bytes(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];

    if (!ValidateRequestId(amx, request_id))
        return 0;

    const cpr::Response& response = g_EasyHttpModule->GetRequest(request_id).response;

    return response.downloaded_bytes;
}

cell AMX_NATIVE_CALL ezhttp_get_user_data(AMX* amx, cell* params)
{
    auto request_id = (RequestId)params[1];
    cell* data_addr = MF_GetAmxAddr(amx, params[2]);

    if (!ValidateRequestId(amx, request_id))
        return 0;

    OptionsId options_id = g_EasyHttpModule->GetRequest(request_id).options_id;
    OptionsData options = g_EasyHttpModule->GetOptions(options_id);

    const std::optional<std::vector<cell>>& user_data = options.user_data;
    if (!user_data)
        return 0;

    MF_CopyAmxMemory(data_addr, user_data.value().data(), user_data.value().size());

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_ftp_upload(AMX* amx, cell* params)
{
    int len;
    std::string user(MF_GetAmxString(amx, params[1], 0, &len));
    std::string password (MF_GetAmxString(amx, params[2], 0, &len));
    std::string host = MF_GetAmxString(amx, params[3], 0, &len);
    std::string remote_file = MF_GetAmxString(amx, params[4], 0, &len);
    int local_file_len;
    char* local_file = MF_GetAmxString(amx, params[5], 1, &local_file_len);
    std::string callback = MF_GetAmxString(amx, params[6], 0, &len);
    bool secure = params[7];
    auto options_id = (OptionsId)params[8];

    std::string url = utils::ConstructFtpUrl(user, password, host, remote_file);

    if (options_id == OptionsId::Null)
        options_id = g_EasyHttpModule->CreateOptions();

    auto& builder = g_EasyHttpModule->GetOptionsBuilder(options_id);
    builder.SetFilePath(MF_BuildPathname("%s", local_file));
    builder.SetSecure(secure);

    SendRequest(amx, RequestMethod::FtpUpload, options_id, url, callback);

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_ftp_upload2(AMX* amx, cell* params)
{
    int url_str_len;
    char* url_str = MF_GetAmxString(amx, params[1], 0, &url_str_len);

    int local_file_len;
    char* local_file = MF_GetAmxString(amx, params[2], 1, &local_file_len);

    int callback_len;
    char* callback = MF_GetAmxString(amx, params[3], 2, &callback_len);

    bool secure = params[4];
    auto options_id = (OptionsId)params[5];

    if (options_id == OptionsId::Null)
        options_id = g_EasyHttpModule->CreateOptions();

    auto& builder = g_EasyHttpModule->GetOptionsBuilder(options_id);
    builder.SetFilePath(MF_BuildPathname("%s", local_file));
    builder.SetSecure(secure);

    SendRequest(amx, RequestMethod::FtpUpload, options_id, std::string(url_str, url_str_len), std::string(callback, callback_len));

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_ftp_download(AMX* amx, cell* params)
{
    int len;
    std::string user(MF_GetAmxString(amx, params[1], 0, &len));
    std::string password (MF_GetAmxString(amx, params[2], 0, &len));
    std::string host = MF_GetAmxString(amx, params[3], 0, &len);
    std::string remote_file = MF_GetAmxString(amx, params[4], 0, &len);
    int local_file_len;
    char* local_file = MF_GetAmxString(amx, params[5], 1, &local_file_len);
    std::string callback = MF_GetAmxString(amx, params[6], 0, &len);
    bool secure = params[7];
    auto options_id = (OptionsId)params[8];

    std::string url = utils::ConstructFtpUrl(user, password, host, remote_file);

    if (options_id == OptionsId::Null)
        options_id = g_EasyHttpModule->CreateOptions();

    auto& builder = g_EasyHttpModule->GetOptionsBuilder(options_id);
    builder.SetFilePath(MF_BuildPathname("%s", local_file));
    builder.SetSecure(secure);

    SendRequest(amx, RequestMethod::FtpDownload, options_id, url, callback);

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_ftp_download2(AMX* amx, cell* params)
{
    int url_str_len;
    char* url_str = MF_GetAmxString(amx, params[1], 0, &url_str_len);

    int local_file_len;
    char* local_file = MF_GetAmxString(amx, params[2], 1, &local_file_len);

    int callback_len;
    char* callback = MF_GetAmxString(amx, params[3], 2, &callback_len);

    bool secure = params[4];
    auto options_id = (OptionsId)params[5];

    auto& builder = g_EasyHttpModule->GetOptionsBuilder(options_id);
    builder.SetFilePath(MF_BuildPathname("%s", local_file));
    builder.SetSecure(secure);

    SendRequest(amx, RequestMethod::FtpDownload, options_id, std::string(url_str, url_str_len), std::string(callback, callback_len));

    return 0;
}

cell AMX_NATIVE_CALL ezhttp_create_queue(AMX* amx, cell* params)
{
    return (cell)g_EasyHttpModule->CreateQueue();
}

cell AMX_NATIVE_CALL ezhttp_steam_to_steam64(AMX* amx, cell* params)
{
    // doc https://developer.valvesoftware.com/wiki/SteamID

    int steam_len;
    char* steam_str = MF_GetAmxString(amx, params[1], 0, &steam_len);
    cell steam64_maxlen = params[3];

    std::string steam(steam_str, steam_len);

    if (steam.find("STEAM_") != 0)
        return 0;

    std::vector<std::string> tokens;
    utils::split(std::string_view(steam.c_str() + sizeof("STEAM_") - 1), ":", tokens);

    if (tokens.size() != 3)
        return 0;

    uint32_t account_id = std::atoi(tokens[2].c_str());    // 32 bit
    account_id = account_id << 1 | std::atoi(tokens[1].c_str());
    uint32_t account_instance = 1;                         // 20 bit, 1 for individual account
    uint8_t account_type = 1;                              // 4 bit,  1 is individual account
    uint8_t universe = 1;                                  // 8 bit, using token[0] produces incorrect results, so use always 1

    uint64_t steam64 = (uint64_t)universe << 56 | (uint64_t)account_type << 52 | (uint64_t)account_instance << 32 | account_id;

    steam.assign(std::to_string(steam64));
    MF_SetAmxString(amx, params[2], steam.c_str(), steam64_maxlen);

    return 1;
}


RequestId SendRequest(AMX* amx, RequestMethod method, OptionsId options_id, const std::string& url, const std::string& callback)
{
    int callback_id = -1;
    if (!callback.empty())
    {
        callback_id = MF_RegisterSPForwardByName(amx, callback.c_str(), FP_CELL, FP_DONE);
        if (callback_id == -1)
        {
            MF_LogError(amx, AMX_ERR_NATIVE, "Callback function \"%s\" is not exists", callback.c_str());
            return RequestId::Null;
        }
    }

    auto on_complete = [callback_id](RequestId request_id) {
        if (callback_id == -1)
        {
            g_EasyHttpModule->DeleteRequest(request_id, true);
            return;
        }

        MF_ExecuteForward(callback_id, request_id);
        MF_UnregisterSPForward(callback_id);

        g_EasyHttpModule->DeleteRequest(request_id, true);
    };

    RequestId request_id = g_EasyHttpModule->SendRequest(method, url, options_id, on_complete);

    return request_id;
}

bool ValidateOptionsId(AMX* amx, OptionsId options_id)
{
    if (!g_EasyHttpModule->IsOptionsExists(options_id))
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Options id %d not exists", options_id);
        return false;
    }

    return true;
}

bool ValidateRequestId(AMX* amx, RequestId request_id)
{
    if (!g_EasyHttpModule->IsRequestExists(request_id))
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Request id %d not exists", request_id);
        return false;
    }

    return true;
}

bool ValidateQueueId(AMX* amx, QueueId queue_id)
{
    if (!g_EasyHttpModule->IsQueueExists(queue_id))
    {
        MF_LogError(amx, AMX_ERR_NATIVE, "Queue id %d not exists", queue_id);
        return false;
    }

    return true;
}

template <class TMethod>
void SetKeyValueOption(AMX* amx, cell* params, TMethod method)
{
    auto options_id = (OptionsId)params[1];
    int key_len;
    char* key = MF_GetAmxString(amx, params[2], 0, &key_len);
    int value_len;
    char* value = MF_GetAmxString(amx, params[3], 1, &value_len);

    if (!ValidateOptionsId(amx, options_id))
        return;

    (g_EasyHttpModule->GetOptions(options_id).options_builder.*method)(std::string(key, key_len), std::string(value, value_len));
}

template <class TMethod>
void SetStringOption(AMX* amx, cell* params, TMethod method)
{
    auto options_id = (OptionsId)params[1];
    int value_len;
    char* value = MF_GetAmxString(amx, params[2], 0, &value_len);

    if (!ValidateOptionsId(amx, options_id))
        return;

    (g_EasyHttpModule->GetOptions(options_id).options_builder.*method)(std::string(value, value_len));
}

AMX_NATIVE_INFO g_Natives[] =
{
    { "ezhttp_create_options",              ezhttp_create_options },
    { "ezhttp_destroy_options",             ezhttp_destroy_options },
    { "ezhttp_option_set_user_agent",       ezhttp_option_set_user_agent },
    { "ezhttp_option_add_url_parameter",    ezhttp_option_add_url_parameter },
    { "ezhttp_option_add_form_payload",     ezhttp_option_add_form_payload },
    { "ezhttp_option_set_body",             ezhttp_option_set_body },
    { "ezhttp_option_append_body",          ezhttp_option_append_body },
    { "ezhttp_option_set_header",           ezhttp_option_set_header },
    { "ezhttp_option_set_cookie",           ezhttp_option_set_cookie },
    { "ezhttp_option_set_timeout",          ezhttp_option_set_timeout },
    { "ezhttp_option_set_connect_timeout",  ezhttp_option_set_connect_timeout },
    { "ezhttp_option_set_proxy",            ezhttp_option_set_proxy },
    { "ezhttp_option_set_proxy_auth",       ezhttp_option_set_proxy_auth },
    { "ezhttp_option_set_auth",             ezhttp_option_set_auth },
    { "ezhttp_option_set_user_data",        ezhttp_option_set_user_data },
    { "ezhttp_option_set_plugin_end_behaviour", ezhttp_option_set_plugin_end_behaviour },
    { "ezhttp_option_set_queue",            ezhttp_option_set_queue },

    { "ezhttp_get",                         ezhttp_get },
    { "ezhttp_post",                        ezhttp_post },
    { "ezhttp_is_request_exists",           ezhttp_is_request_exists },
    { "ezhttp_cancel_request",              ezhttp_cancel_request },
    { "ezhttp_request_progress",            ezhttp_request_progress },

    { "ezhttp_get_http_code",               ezhttp_get_http_code },
    { "ezhttp_get_data",                    ezhttp_get_data },
    { "ezhttp_get_url",                     ezhttp_get_url },
    { "ezhttp_save_data_to_file",           ezhttp_save_data_to_file },
    { "ezhttp_save_data_to_file2",          ezhttp_save_data_to_file2 },
    { "ezhttp_get_headers_count",           ezhttp_get_headers_count },
    { "ezhttp_get_headers",                 ezhttp_get_headers },
    { "ezhttp_iterate_headers",             ezhttp_iterate_headers },
    { "ezhttp_get_elapsed",                 ezhttp_get_elapsed },
    { "ezhttp_get_cookies_count",           ezhttp_get_cookies_count },
    { "ezhttp_get_cookies",                 ezhttp_get_cookies },
    { "ezhttp_iterate_cookies",             ezhttp_iterate_cookies },
    { "ezhttp_get_error_code",              ezhttp_get_error_code },
    { "ezhttp_get_error_message",           ezhttp_get_error_message },
    { "ezhttp_get_redirect_count",          ezhttp_get_redirect_count },
    { "ezhttp_get_uploaded_bytes",          ezhttp_get_uploaded_bytes },
    { "ezhttp_get_downloaded_bytes",        ezhttp_get_downloaded_bytes },
    { "ezhttp_get_user_data",               ezhttp_get_user_data },

    { "ezhttp_ftp_upload",                  ezhttp_ftp_upload },
    { "ezhttp_ftp_upload2",                 ezhttp_ftp_upload2 },
    { "ezhttp_ftp_download",                ezhttp_ftp_download },
    { "ezhttp_ftp_download2",               ezhttp_ftp_download2 },

    { "ezhttp_create_queue",                ezhttp_create_queue },

    { "_ezhttp_steam_to_steam64",           ezhttp_steam_to_steam64 },

    { nullptr,                              nullptr },
};

void OnAmxxAttach()
{
    g_EasyHttpModule = std::make_unique<EasyHttpModule>(MF_BuildPathname("addons/amxmodx/data/amxx_easy_http_cacert.pem"));

    MF_AddNatives(g_Natives);
}

void OnAmxxDetach()
{
    g_EasyHttpModule = nullptr;
}

void StartFrame()
{
    if (g_EasyHttpModule)
        g_EasyHttpModule->RunFrame();

    SET_META_RESULT(MRES_IGNORED);
}

void ServerDeactivate()
{
    if (g_EasyHttpModule)
        g_EasyHttpModule->ServerDeactivate();

    SET_META_RESULT(MRES_IGNORED);
}

void GameShutdown()
{
    if (g_EasyHttpModule)
        g_EasyHttpModule = nullptr;
}
