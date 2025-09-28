_ini ini;

void LoadIniConfig()
{
    ini.path = GetIni(#settings_file, "app.ini");
    ini.section = "WebView";

    ini.GetString("proxy", #proxy_address, sizeof(proxy_address), NULL);
}