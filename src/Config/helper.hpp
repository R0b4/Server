constexpr int amount_config_commands = 9;
enum config_t : int {
    cchunksize = 0, cinitbuffsize, creallocthres, cmaxconn, 
    cport, cbacklog, cusessl, csslport, csslbacklog
};

std::map<config_t, string_view> config_commands = {
    {cchunksize, string_view("Send chunk size")}, 
    {cinitbuffsize, string_view("initial buffer size")}, 
    {creallocthres}
};

void run_config_helper() {

}