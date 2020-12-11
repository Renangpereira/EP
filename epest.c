#include "include/epest.h"
#include "include/eppng.h"

ep_filesystem *ep;

#include "ep.c"
#include "epfuse.c"

int main(int argc, char *argv[]) {
    char actualpath [MAX_PATH];
    char* base;
    int8_t fargc = 6;
    const char *fargv[fargc];
    char key_input[4096];

    // Verificação minima de argc and root
    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Will not run as root!!\n");
        exit(1);
    }
    if (argc < 3) { ep_usage(); }

    //  Opções de ajuda/debug 
    ep_dbg = 0;
    if(parseArgv(argc, argv, DEBUG_OPTION)) { ep_dbg = 1; }
    if(parseArgv(argc, argv, HELP_OPTION)) { ep_usage(); }

    // Pega o caminho absoluto, img_dir path, e nomedabase
    realpath(argv[argc - 1], actualpath);
    base = basename(actualpath);
    ep = malloc(sizeof(ep_filesystem));
    ep->root_img = malloc(sizeof(png_data));
    strncpy(ep->img_dir, actualpath, strlen(actualpath) - strlen(base));
    strcpy(ep->root_img->filename, base);

    // lendo a imagem raiz
    if (!read_png(ep->root_img, ep->img_dir)) { ep_usage(); }

    // Nome do Drive
    printf("Digite o nome do Drive:\n");
    fgets(ep->name, sizeof(ep->name), stdin);
    ep->name[strlen(ep->name) - 1] = '\0';
    strcat(ep->name, ".ep");

    // chave
    printf("Coloque a chave de criptografia %s:\n", ep->name);
    fgets(key_input, sizeof(key_input), stdin);
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, key_input, strlen(key_input));
    SHA512_Final((unsigned char *)ep->key, &sha512);
    printf("\e[1;1H\e[2J");

    // Verificar a mountagem
    if (parseArgv(argc, argv, MNT_OPTION)) { readRoot(); }
    // Verifica formt
    else if (parseArgv(argc, argv, FORMAT_OPTION)) { ep_format(); }
    // Verifica expand
    else if (parseArgv(argc, argv, EXPAND_OPTION)) { ep_expand(); }
    else { ep_usage(); }

    // Fuse
    fargv[0] = argv[0];
    fargv[1] = "-f";
    fargv[2] = "-o";
    fargv[3] = "big_writes";
    fargv[4] = "-s";
    fargv[5] = ep->name;
    mkdir(ep->name, 0755);
    printf("\n%s Montado.\nepest continuará a correr em primeiro plano.\nUse Ctrl+C para Desmontar o sistema de arquivos de modo seguro.\n", ep->name);
    return fuse_main(fargc, (char **)fargv, &ep_oper, NULL);
}


/* help e exit */
void ep_usage() {
    fprintf(stderr, "epest [-help] [-debug] [-expand] [-format] [-mount] <path/to/root.png>\n");
    exit(1);
}


/* Analisa as opções passadas para uma determinada opção e retorna sua posição em argv */
int parseArgv(int argc, char *argv[], char *option) {
    int8_t length = strlen(option);
    int8_t y;
    for(y = 0; y < argc; y++) {
        if(strncmp(argv[y], option, length) == 0) { return y; }
    }
    return 0;
}


/* Calcula MD5 do arquivo*/
int getMD5(char *filename, char *md5_sum) {
    char path[MAX_PATH];
    strcpy(path, ep->img_dir);
    strcat(path, filename);
    FILE *f = fopen(path, "rb");
    char data[sizeof(md5_sum)];

    if (f == NULL) { return 0; }
    MD5_CTX mdContext;
    MD5_Init (&mdContext);
	while (fread (data, 1, sizeof(md5_sum), f) != 0) {
            MD5_Update (&mdContext, data, sizeof(md5_sum));
        }
    MD5_Final ((unsigned char *)md5_sum, &mdContext);
    pclose(f);
    return 1;
}
