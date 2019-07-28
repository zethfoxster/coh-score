extern int err;
extern char glob_map_name[];
extern bool g_verbose_client;

void processCmd(char *buf, int quiet);
int error_exit(int allow_debug);