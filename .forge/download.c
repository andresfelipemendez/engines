#include <curl/curl.h>

static size_t write_file_cb(void *ptr, size_t sz, size_t nm, void *userdata) {
    FILE *f = (FILE*)userdata;
    return fwrite(ptr, sz, nm, f);
}

// returns 0 on success, nonzero on error
int https_download(const char *url, const char *out_path) {
    int rc = 1;
    CURL *curl = NULL;
    FILE *f = fopen(out_path, "wb");
    if (!f) return rc;

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) { fclose(f); return rc; }
    curl = curl_easy_init();
    if (!curl) { fclose(f); curl_global_cleanup(); return rc; }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "forge/0.1");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) rc = 0;

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    fclose(f);
    if (rc) remove(out_path);
    return rc;
}
