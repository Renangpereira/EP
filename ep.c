/* 
  Programa: EP
  Nome do arquivo: ep.c

  SISTEMAS OPERACIONAIS
  
*/

// Lendo um bit de dados de 1 byte em ep 
int readBit(png_bytep row, int64_t x_offset, int8_t pixel) {
    png_bytep px = &(row[x_offset * 3]);
    return (px[pixel] & 1);
}

/* Lendo um bite de dados de 8 bytes in ep */
char readByte(int64_t offset) {
    unsigned char byte;
    int8_t x, bit, pixel, root;
    int32_t img_row_num, img_num, row_num, x_offset;
    if (offset < 0) { 
        root = 1; 
        offset = abs(offset);
    } else { root = 0; }

    pixel = offset % 3;
    x_offset = ((int) offset / 3) % ep->root_img->width;
    row_num = (offset / 3) / ep->root_img->width;
    img_num = row_num / ep->root_img->height;
    img_row_num = row_num % ep->root_img->height;

    if (ep_dbg) { printf("#Byte Pronto\n"); }
    for (x = 8; x > 0; x--) {
        if (!root) { bit = readBit(ep->images[img_num]->row_pointers[img_row_num], x_offset, pixel); }
        else { bit = readBit(ep->root_img->row_pointers[img_row_num], x_offset, pixel); }
        if (ep_dbg) { 
            if (!root) { printf("%d | %d(%d, %d):%d\n", bit, img_num, x_offset, img_row_num, pixel); }
            else { printf("%d | -1(%d, %d):%d\n", bit, x_offset, img_row_num, pixel); }
        }
        if (bit) { byte |= 1; }
        else { byte &= ~1; }
        if (x != 1) { byte <<= 1; }
        if (x_offset == (ep->root_img->width - 1) && pixel == 2) {
            if ((img_row_num == (ep->root_img->height - 1)) && !root) {
                img_num++;
                img_row_num = 0;
            }
            else { img_row_num++; }
            x_offset = 0;
            if ((img_num) > ep->img_count || (root && img_row_num == (ep->root_img->height - 1))) {
                fprintf(stderr, "tentando ler fora dos limites em %s dentro de %d\n", ep->name, img_num);
                break;
            }
        }
        if (pixel == 2) { 
            pixel = 0;
            x_offset++; 
        }
        else { pixel++; }
    }
    byte ^= ep->key[offset % strlen(ep->key)];
    return byte;
}


/* Lendo um bit de dados de n*8 bytes in ep */
int readBytes(char *buf, size_t size, off_t offset) {
    int i;

    if ((((offset + size) > ep->capacity) && (offset > 0)) || ((offset < 0) && ((abs(offset) + size) > ceil((ep->root_img->height * ep->root_img->width * 3) / 8)))) {
        fprintf(stderr, "Falha na leitura, filesystem %s está fora dos limites", ep->name);
        return -1;
    }
    for (i = 0; i < size; i++) {
        buf[i] = readByte(offset * 8);
        if (offset >= 0) { offset++; }
        else { offset--; }
    }
    return i;
}

/* Escrita de 1 bite de dados de 1 byte em ep */
void writeBit(png_bytep row, int64_t x_offset, int8_t pixel, int8_t bit) {
    png_bytep px = &(row[x_offset * 3]);
    if (bit) { px[pixel] |= 1; }
    else { px[pixel] &= ~1; }
}


/* Escrita de 1 byte de dados de 8 bytes em ep */
void writeByte(char *b, int64_t offset) {
    int root = 0;
    int8_t pixel, y, x, bit[8];
    int32_t img_row_num, img_num, row_num, x_offset;
    char byte;

    if (offset < 0) {
        root = 1; 
        offset = abs(offset);
    } else { root = 0; }
    strncpy(&byte, b, sizeof(char));
    byte ^= ep->key[offset % strlen(ep->key)];

    pixel = offset % 3;
    x_offset = ((int) offset / 3) % ep->root_img->width;
    row_num = (offset / 3) / ep->root_img->width;
    img_num = row_num / ep->root_img->height;
    img_row_num = row_num % ep->root_img->height;

    if (ep->images[img_num]->state != modified) { ep->images[img_num]->state = modified; }
    for (y = 0; y < 8; y++) {
        bit[y] = (byte & 1);
        byte >>= 1;
    }
    if (ep_dbg) { printf("# Byte escrito\n"); }
    for (x = 8; x > 0; x--) {
        if (ep_dbg) { 
            if (!root) { printf("%d | %d(%d, %d(%d)):%d\n", bit[x - 1], img_num, x_offset, img_row_num, row_num, pixel); }
            else { printf("%d | -1(%d, %d(%d)):%d\n", bit[x - 1], x_offset, img_row_num, row_num, pixel); }
        }
        if (!root) { writeBit(ep->images[img_num]->row_pointers[img_row_num], x_offset, pixel, bit[x - 1]); }
        else { writeBit(ep->root_img->row_pointers[img_row_num], x_offset, pixel, bit[x - 1]); }
        if (x_offset == (ep->root_img->width - 1) && pixel == 2) {
            if ((img_row_num == (ep->root_img->height - 1)) && !root) {
                img_num++;
                img_row_num = 0;
            }
            else { img_row_num++; }
            x_offset = 0;
            if ((img_num) > ep->img_count || (root && img_row_num == (ep->root_img->height - 1))) {
                fprintf(stderr, "tentando ler fora dos limites em %s dentro de %d\n", ep->name, img_num);
                return;
            }
        }
        if (pixel == 2) { 
            pixel = 0;
            x_offset++; 
        }
        else { pixel++; }
    }
    return;
}

/* Gravando n bytes de dados em n * 8 bytes em ep */
int writeBytes(char *buf, size_t size, off_t offset) {
    int i;

    if ((((offset + size) > ep->capacity) && (offset > 0)) || ((offset < 0) && ((abs(offset) + size) > ceil((ep->root_img->height * ep->root_img->width * 3) / 8)))) {
        fprintf(stderr, "Falha de escrita, tentando escrever além da memória!\no: %d s: %d\n", (int)offset, (int)size);
        return -1;
    }
    for (i = 0; i < size; i++) {
        writeByte(&buf[i], (offset * 8));
        if ((int)offset >= 0) { 
            offset = offset + 1;; 
            ep->consumed++;;
        }
        else { offset = offset - 1; }
    }
    return i;
}


/* Limpando um bit de dados de 1 byte em ep */
void wipeBit(png_bytep row, int64_t x_offset, int8_t px_chan) {
    png_bytep px = &(row[x_offset * 3]);
    px[px_chan] &= ~1;
}


/* Limpando um bit de dados de 8 bytes em ep */
void wipeByte(int64_t offset) {
    int root = 0;
    int8_t pixel, x;
    int32_t img_row_num, img_num, row_num, x_offset;

    if (offset < 0) {
        root = 1;
        offset = abs(offset);
    } else { root = 0; }

    pixel = offset % 3;
    x_offset = ((int) offset / 3) % ep->root_img->width;
    row_num = (offset / 3) / ep->root_img->width;
    img_num = row_num / ep->root_img->height;
    img_row_num = row_num % ep->root_img->height;

    if (ep->images[img_num]->state != modified) { ep->images[img_num]->state = modified; }
    if (ep_dbg) { printf("# Wipe byte\n"); }
    for (x = 8; x > 0; x--) {
        if (ep_dbg) {
            if (!root) { printf("0 | %d(%d, %d(%d)):%d\n", img_num, x_offset, img_row_num, row_num, pixel); }
            else { printf("0 | -1(%d, %d(%d)):%d\n", x_offset, img_row_num, row_num, pixel); }
        }
        if (!root) { wipeBit(ep->images[img_num]->row_pointers[img_row_num], x_offset, pixel); }
        else { wipeBit(ep->root_img->row_pointers[img_row_num], x_offset, pixel); }
        if (x_offset == (ep->root_img->width - 1) && pixel == 2) {
            if ((img_row_num == (ep->root_img->height - 1)) && !root) {
                img_num++;
                img_row_num = 0;
            }
            else { img_row_num++; }
            x_offset = 0;
            if ((img_num) > ep->img_count || (root && img_row_num == (ep->root_img->height - 1))) {
                fprintf(stderr, "tentando ler fora dos limites da memória %s em %d\n", ep->name, img_num);
                return;
            }
        }
        if (pixel == 2) {
            pixel = 0;
            x_offset++;
        }
        else { pixel++; }
    }
}

/* Limpando n bytes da memória*/
int wipeBytes(ep_file *file) {
    int i;
    off_t offset = file->offset;

    for (i = 0; i < file->size + 1; i++) {
        wipeByte(offset * 8);
        ep->consumed--;
        offset++;
    }
    return i;
}

/* Criação do arquivo */
int createFile(char *path) {
    int i;

    ep_file **files = malloc(sizeof(ep_file) * (ep->file_count + 1));
    for (i = 0; i < (ep->file_count); i++) { files[i] = ep->files[i]; }
    free(ep->files);
    ep->files = files;
    ep_file *newfile = malloc(sizeof(ep_file));
    strcpy(newfile->name, path);
    newfile->size = 0;
    newfile->state = created;
    if (ep->file_count == 0) { newfile->offset = 0; }
    else {
        newfile->offset = (ep->files[ep->file_count - 1]->offset + ep->files[ep->file_count - 1]->size + 1);
    }
    ep->files[ep->file_count] = newfile;
    ep->file_count++;
    return 0;
}

/* Arquivo de escrita */
int writeFile(char *path, char *buf, size_t size, off_t offset) {
    int result = 0;
    int f = findFile(path);

    if (size > (int)(ep->capacity - ep->consumed)) { return -ENOSPC; }
    if (f == -1) { return -ENOENT; }
    ep_file *file = ep->files[f];

    if (file->state == created && (f == (ep->file_count - 1))) {
        file->size = size;
        file->state = is_file;
        result = writeBytes(buf, file->size, file->offset);
    }
    else if (file->state == is_file && (f == (ep->file_count - 1))) {
        if (file->size < size + offset) {
            result = writeBytes(buf, size, file->offset + file->size);
            file->size = file->size + size;
        }
        else { result = writeBytes(buf, size, offset); }
    }
    else {
        fprintf(stderr, "Tentando gravar em um arquivo inválido %s\n", ep->files[f]->name);
        result = 0;
    }
    return result;
}


/* Read file */
int readFile(char *path, char *buf, size_t size, off_t offset) {
    int result = 0;
    int f = findFile(path);

    if (f == -1) { return -ENOENT; }
    ep_file *file = ep->files[f];
    if (file->size > offset) {
        result = readBytes(buf, size, file->offset + offset);
    }
    else { return 0; }
    return result;
}


/* Wipe file */
int wipeFile(char *path) {
    int result = 0;
    int f = findFile(path);
    int x;

    if (f == -1) { return -ENOENT; }
    ep_file *deleted_file = ep->files[f];

    result += wipeBytes(deleted_file);
    if (f != (ep->file_count - 1)) { 
        ep_file **new_files = malloc(sizeof(ep_file) * ep->file_count - 1);
        for (x = 0; x < f; x++) { new_files[x] = ep->files[x]; }
        for (x = 0; x < ep->file_count - f - 1; x++) {
            char *tmp = calloc(1, ep->files[(f + x + 1)]->size);
            ep_file *new_file = malloc(sizeof(ep_file));
            readBytes(tmp, ep->files[(f + x + 1)]->size, ep->files[(f + x + 1)]->offset);
            result += wipeBytes(ep->files[(f + x + 1)]);
            if (x == 0) { new_file->offset = ep->files[(f + x)]->offset; }
            else { new_file->offset = new_files[(f + x - 1)]->offset + new_files[(f + x - 1)]->size + 1; }
            strcpy(new_file->name, ep->files[(f + x + 1)]->name);
            new_file->size = ep->files[(f + x + 1)]->size;
            writeBytes(tmp, new_file->size, new_file->offset);
            new_files[f + x] = new_file;
            free(ep->files[(f + x + 1)]);
            free(tmp);
        }
        free(ep->files);
        ep->files = new_files;
    }
    else {
        free(ep->files[f]);
    }
    ep->file_count--;
    return result;
}

/* Encontre os metadados de um arquivo em seu caminho */
int findFile(char *path) {
    int y;

    for (y = 0; y < ep->file_count; y++) {
        if (strcmp(ep->files[y]->name, path) == 0) { return y; }
    }
    return -1;
}

/* Exapandindo filystyem existente */
void ep_expand() {
    DIR *FD;
    struct dirent *dir;
    int x, y, exists;
    int added = 0;

    readRoot();
    FD = opendir(ep->img_dir);
    if (!FD) {
        fprintf(stderr, "Não é possivel abrir diretório %s", ep->img_dir);
        free(ep);
        exit(1);
    }
    while ((dir = readdir(FD)) != NULL) {
        if (strncmp(dir->d_name + strlen(dir->d_name) - 4, ".png", 4) == 0) {
            exists = 0;
            for (x = 0; x < ep->img_count; x++) {
                if ((strcmp(dir->d_name, ep->images[x]->filename) == 0) || (strcmp(ep->root_img->filename, dir->d_name) == 0)) { exists = 1; }
            }
            if (!exists) {
                png_data **new_images = malloc(sizeof(png_data) * ep->img_count + 1);
                for (y = 0; y < ep->img_count; y++) { new_images[y] = ep->images[y]; }
                png_data *new_img = malloc(sizeof(png_data));
                strcpy(new_img->filename, dir->d_name);
                if (!read_png(new_img, ep->img_dir)) {
                    free(new_img);
                    free(new_images);
                }
                else if (new_img->height != ep->root_img->height || new_img->width != ep->root_img->width) {
                    if (ep_dbg) { printf("%s Não tem o mesmo tamanho da imagem raiz\n", dir->d_name); }
                    free(new_img);
                    free(new_images);
                }
                else {
                    clearAllLSB(new_img);
                    ep->capacity += ((new_img->width * new_img->height * 3) / 8);
                    new_images[ep->img_count++] = new_img;
                    free(ep->images);
                    ep->images = new_images;
                    added++;
                }
            }
        }
        ep_file **files = malloc(sizeof(ep_file*) * ep->file_count);
        ep->files = files;
    }
    if (added) {
        printf("%d Adicionada imagem em %s\n", added, ep->name);
        printf("#### Filesystem Atualizado ####\n");
        printf("Nome       : %s\n", ep->name);
        printf("PNG Count  : %d\n", ep->img_count);
        printf("Capacidade   : %.2f MB\n", ((ep->capacity / 1000) / 1000));
        printf("Consumo   : %.2f MB\n", ((ep->consumed / 1000) / 1000));
    }
    else { printf("Sem novas imagens, filesystem não expandido.\n"); }
}

/* Formatando novo filesystem */
void ep_format() {
    DIR *FD;
    struct dirent *dir;
    int png_cnt, valid_png_cnt, y;

    FD = opendir(ep->img_dir);
    if (!FD) {
        fprintf(stderr, "Diretório não aberto %s", ep->img_dir);
        free(ep);
        exit(1);
    }
    png_cnt = 0;
    while ((dir = readdir(FD)) != NULL) {
        if (strncmp(dir->d_name + strlen(dir->d_name) - 4, ".png", 4) == 0) {
            png_cnt++;
        }
    }
    seekdir(FD, 0);
    png_data **dir_images = malloc(sizeof(png_data*) * png_cnt);
    valid_png_cnt = 0;
    while ((dir = readdir(FD)) != NULL) {
        if (strncmp(dir->d_name + strlen(dir->d_name) - 4, ".png", 4) == 0) {
            png_data *new_img = malloc(sizeof(png_data));
            strcpy(new_img->filename, dir->d_name);
            if (!read_png(new_img, ep->img_dir)) {
                printf("%s not valid read\n", dir->d_name);
                free(new_img);
            }
            else if (new_img->height != ep->root_img->height || new_img->width != ep->root_img->width) {
                if (ep_dbg) { printf("%s not same size as root image\n", dir->d_name); }
                free(new_img);
            }
            else if (strcmp(ep->root_img->filename, new_img->filename) == 0) {
                free(new_img);
            }
            else {
                clearAllLSB(new_img);
                dir_images[valid_png_cnt] = new_img;
                valid_png_cnt++;
            }
        }
    }
    closedir(FD);
    if (valid_png_cnt < MINIMUM_PNG) {
        fprintf(stderr, "Sem imagens suficientes %s para criar %s. Pelo menos %d imagens em PNG é necessário.\n", ep->img_dir, ep->name, MINIMUM_PNG);
        if (valid_png_cnt) {
        fprintf(stderr, "Imagens atuais validas:\n");
            for (y = 0; y < valid_png_cnt; y++) {
                fprintf(stderr, "%s\n", dir_images[y]->filename);
            }
        }
        else { fprintf(stderr, "Nenhuma outra imagem válida encontrada.\n"); }
        exit(1);
    }
    ep->img_count = valid_png_cnt;
    ep->images = dir_images;
    ep->capacity = ceil(((valid_png_cnt * ep->root_img->height * ep->root_img->width * 3) / 8));
    ep->consumed = 0;
    printf("#### Novo Filesystem ####\n");
    printf("Nome       : %s\n", ep->name);
    printf("PNG Count  : %d\n", ep->img_count);
    printf("Capacidade   : %.2f MB\n", ((ep->capacity / 1000) / 1000));
    printf("Consumo   : %.2f MB\n", ((ep->consumed / 1000) / 1000));
    for (y = 0; y < ep->img_count; y++) {
        if (ep_dbg) { printf("%d: %s\n", y, ep->images[y]->filename); } 
    }
}


/* Grave alguns filesystem_meta e png_data para fazer root img */
void writeRoot() {
    int y;
    int offset = -1;

    // Write filesystem name, consumed, capacity and image count
    clearAllLSB(ep->root_img);
    writeBytes((void *) ep->name, sizeof(ep->name), offset);
    offset = offset - sizeof(ep->name);
    writeBytes((void *) &ep->consumed, sizeof(float), offset);
    offset = offset - sizeof(float);
    writeBytes((void *) &ep->capacity, sizeof(float), offset);
    offset = offset - sizeof(float);
    writeBytes((void *) &ep->img_count, sizeof(int32_t), offset);
    offset = offset - sizeof(int32_t);
    writeBytes((void *) &ep->file_count, sizeof(int32_t), offset);
    offset = offset - sizeof(int32_t);
    
    
    // Write png_data for each valid image
    for (y = 0; y < ep->img_count; y++) {
        getMD5(ep->images[y]->filename, ep->images[y]->md5);
        writeBytes((void *) ep->images[y]->md5, sizeof(ep->images[y]->md5), offset);
        offset = offset -sizeof(ep->images[y]->md5);
    }

    // Grave arquivo_meta para cada arquivo
    for (y = 0; y < ep->file_count; y++) {
        writeBytes((void *) ep->files[y], sizeof(ep_file), offset);
        offset = offset - sizeof(ep_file);
    }
    write_png(ep->root_img, ep->img_dir);
}


/* Leia filesystem_meta e png_data do root img */
void readRoot() {
    DIR *FD;
    struct dirent *dir;
    int y;
    int offset = -1;
    char name[64];

    // Ler o nome do sistema de arquivos, consumido, capacidade e contagem de imagens
    readBytes((void *) name, sizeof(ep->name), offset);
    offset = offset - sizeof(ep->name);
    if (strncmp(name, ep->name, (strlen(ep->name))) != 0) {
        fprintf(stderr, "Não é possível ler o sistema de arquivos! A chave, o nome ou a imagem raiz fornecidos estão incorretos.\n");
        exit(1);
    }
    readBytes((void *) &ep->consumed, sizeof(float), offset);
    offset = offset - sizeof(float);
    readBytes((void *) &ep->capacity, sizeof(float), offset);
    offset = offset - sizeof(float);
    readBytes((void *) &ep->img_count, sizeof(int32_t), offset);
    offset = offset - sizeof(int32_t);
    readBytes((void *) &ep->file_count, sizeof(int32_t), offset);
    offset = offset - sizeof(int32_t);
    printf("Sistema de arquivos encontrado %s [%0.2f/%0.2f] %d files in %d images\n", name, ep->consumed, ep->capacity, ep->file_count, ep->img_count);

    png_data **dir_images = malloc((sizeof(png_data*) * ep->img_count));
    FD = opendir(ep->img_dir);
    if (!FD) {
        fprintf(stderr, "Não é possível abrir o diretório %s", ep->img_dir);
        free(ep);
        exit(1);
    }

    // Leia png_data para cada imagem válida
    for (y = 0; y < ep->img_count; y++) {
        png_data *new_img = malloc(sizeof(png_data));
        char tmp[strlen(new_img->md5)];
        readBytes((void *) new_img->md5, sizeof(new_img->md5), offset);
        offset = offset - sizeof(new_img->md5);
        while ((dir = readdir(FD)) != NULL) {
            memset(tmp, 0, sizeof(new_img->md5));
            getMD5(dir->d_name, tmp);;
            if (strncmp(tmp, new_img->md5, sizeof(new_img->md5)) == 0) {
                strcpy(new_img->filename, dir->d_name);
                if (!read_png(new_img, ep->img_dir)) {
                    fprintf(stderr, "Falta imagem no sistema de arquivo %s!", new_img->filename);
                    exit(1);
                }
                dir_images[y] = new_img;
                break;
            }
        }
        seekdir(FD, 0);
    }
    closedir(FD);
    ep->images = dir_images;

    ep_file **files = malloc((sizeof(ep_file*) * ep->file_count));

    // Lendo cada arquivo_meta para cada arquivo
    for (y = 0; y < ep->file_count; y++) {
        ep_file *new_file = malloc(sizeof(ep_file));
        readBytes((void *) new_file, sizeof(ep_file), offset);
        offset = offset - sizeof(ep_file);
        files[y] = new_file;
    }
    ep->files = files;
    if (ep_dbg) { printf("Filesystem %s carregado\n", ep->name); }
}


/* Limpe LSB em cada R, G, B*/
void clearAllLSB(png_data *img) {
    int cnty, cntx, px_chan;
    for(cnty = 0; cnty < img->height; cnty++) {
        png_bytep row = img->row_pointers[cnty];
        for(cntx = 0; cntx < img->width; cntx++) {
            png_bytep px = &(row[cntx * 3]);
            for(px_chan = 0; px_chan < 3; px_chan++) {
                px[px_chan] &= ~1;
            }
        }
    }
    img->state = modified;
}

/* Grave o estado nas imagens modificadas e saia*/
void saveState() {
    int i;
    for (i = 0; i < ep->img_count; i++) {
        if (ep->images[i]->state == modified) {
            write_png(ep->images[i], ep->img_dir);
            ep->images[i]->state = not_modified;
        }
    }
    writeRoot();
    rmdir(ep->name);
    printf("\nDesmontagem feita com sucesso %s, goodbye!\n", ep->name);
}
