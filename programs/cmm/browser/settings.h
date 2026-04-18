_ini ini;

void LoadIniConfig()
{
    ini.path = GetIni(#settings_file, "app.ini");
    ini.section = "WebView";

    ini.GetString("proxy_address", #proxy_address, sizeof(proxy_address), NULL);
    ini.GetString("search_engine", #search_engine, sizeof(search_engine), "http://bing.com/search?q=");
}