_ini ini;

void LoadIniConfig()
{
    ini.path = GetIni("/sys/settings/app.ini", "app.ini");
    ini.section = "WebView";

    ini.GetString("proxy", #proxy_address, sizeof(proxy_address), NULL);
}