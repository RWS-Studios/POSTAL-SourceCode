#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



#ifdef WIN32
#include <direct.h>
#define mkdir(x, y) _mkdir(x)
#define mkstemp(x) _mktemp_s(x, 4096)

#include <sys/types.h>
#endif

#include <sys/stat.h>
#include "w_dirent.h"

#define VER_MAJ     0
#define VER_MIN     1


#define SAK_COOKIE 0x204b4153		// Looks like "SAK " in the file
#define SAK_CURRENT_VERSION 1		// Current version of SAK file format

typedef struct  {
    char*       name;
    uint32_t     offset;
    uint32_t     size;
    int32_t      from_sak;
} sak_record;

typedef struct {
    uint32_t         sig;
    uint32_t         ver;
    uint32_t         size;
    uint32_t         cap;
    sak_record       *records;
} sak_file;

void read_cstring(FILE*f, char* buff) {
    int i = 0;
    do {
        fread(buff+i, 1, 1, f);
    } while (buff[i++]);
}

// return 1 if name is a match for mask, return 0 if not (mask can include ? and *)
int match(const char* name, const char* mask)
{
    int i = 0, j = 0;
    if(!mask) return 1;     // empty mask is always a match
    if(!mask[0]) return 1;
    
    char c, c1, cn;
    c1 = mask[j++]; cn = mask[j++];
    do {
        c = name[i++];
        if (c!=c1) {
            if (c1=='?') {
                c1 = cn;
                cn = mask[j++];
            } else if (c1!='*') {
                return 0;
            } else {
                if (c == cn) {
                    c1 = mask[j++];
                    cn = mask[j++];
                }
            }
        } else {
            c1 = cn;
            cn = mask[j++];
        }
    } while (c);

    return 1;
}

// very rudimentary folder check
int check_folder(const char* folder)
{
    if(!folder) 
        return 0;
    if(!folder[0])
        return 0;
    if(strchr(folder,'*')!=NULL)
        return 0;
    if(strchr(folder,'?')!=NULL)
        return 0;
    
    return 1;
}

int create_folder(const char* folder)
{
    // recursively create folders
    struct stat st = {0};
    char* buff = strdup(folder);
    char* name = strrchr(buff, '/');
    if(name) {
        *name='\0';
        name++;
        create_folder(buff);
    } else 
        name = buff;

    if(stat(folder, &st) == -1) {
        mkdir(folder, 0777);    //TODO, put some better permissions
        if(stat(folder, &st) == -1) {
            free(buff);
            return 0;
        }
    } 
    free(buff);
    return 1;
}

void add_offset(uint32_t *offsets, uint32_t off, int size)
{
    int i = 0;
    do {
        if(!offsets[i]) {
            offsets[i] = off;
            return;
        }
        if(offsets[i]>=off) {
            for (int j=size-1; j>i; j--) offsets[j]=offsets[j-1];
            offsets[i] = off;
            return;
        }
        i++;
    } while(i<size);
}

uint32_t size_from_offset(uint32_t *offsets, uint32_t off, int size)
{
    int i = 0;
    do {
        if(offsets[i]==off) {
            return offsets[i+1] - offsets[i];
        }
        i++;
    } while (i<size);
    return 0;
}

sak_file* read_sak(const char* sakname)
{
    sak_file * sak = (sak_file*)malloc(sizeof(sak_file));
    memset(sak, 0, sizeof(sak_file));
    sak->sig = SAK_COOKIE;
    sak->ver = SAK_CURRENT_VERSION;
    // open sak file
    FILE *sakfile = fopen(sakname, "rb");
    if(sakfile==NULL) {
        // nothing to read...
        printf("New file\n");
        return sak;
    }
    // read & check signature
    uint32_t ul;
    fread(&ul, sizeof(ul), 1, sakfile);
    if(ul != SAK_COOKIE) {
        printf("Error: not a SAK file\n");
        free(sak);
        return NULL;
    }
    // read & check version
    fread(&ul, sizeof(ul), 1, sakfile);
    if(ul != SAK_CURRENT_VERSION) {
        printf("Error: SAK version (%u) unsuported\n", ul);
        free(sak);
        return NULL;
    }
    uint16_t us;
    fread(&us, sizeof(us), 1, sakfile);
    sak->cap = us;
    sak->records = (sak_record*)malloc(us*sizeof(sak_record));
    uint32_t n = 0, offset = 0;
    char buff[256];
    while (n<us) {
        read_cstring(sakfile, buff);
        sak->records[n].name = strdup(buff);
        fread(&ul, sizeof(ul), 1, sakfile);
        sak->records[n].offset = ul;
        sak->records[n].from_sak = 1;
        n++;
    }
    fseek(sakfile, 0, SEEK_END);
    ul = ftell(sakfile);
    // create an ordered maps of offset
    uint32_t *offsets = (uint32_t*)malloc((n+1)*sizeof(uint32_t));
    memset(offsets, 0, sizeof(uint32_t)*(n+1));
    for (int j=0; j<n; j++) add_offset(offsets, sak->records[j].offset, n+1);
    add_offset(offsets, ul, n+1);
    for (int j=0; j<n; j++) sak->records[j].size = size_from_offset(offsets, sak->records[j].offset, n+1);

    sak->size = n;

    fclose(sakfile);

    return sak;
}

void list_sak_content(sak_file *sak, const char* mask)
{
    if(!sak) return;
    printf("SAK version %u\n\n", sak->ver);
    for (int i=0; i<sak->size; i++)
    {
        if(match(sak->records[i].name, mask))
            printf(" %s\tsize=%u\n", sak->records[i].name, sak->records[i].size);
    }
}

void extract_sak(const char* sakfile, sak_file *sak, const char* mask, const char* folder)
{
    if(!sak) return;
    char name[4096];
    void* buff = NULL;
    uint32_t size = 0;
    FILE *f = fopen(sakfile, "rb");
    for (int i=0; i<sak->size; i++) {
        if(match(sak->records[i].name, mask)) {
            strcpy(name, folder);
            strcat(name, "/");
            strcat(name, sak->records[i].name);
            printf("Extracting %s...", name);
            // read data
            if(sak->records[i].size > size) {
                free(buff);
                size = sak->records[i].size;
                buff = malloc(size);
            }
            fseek(f, sak->records[i].offset, SEEK_SET);
            fread(buff, sak->records[i].size, 1, f);
            // try to create file
            FILE *o = fopen(name, "wb");
            if (o==NULL) {
                // faillure, try to locate any folder and create them
                char* tmp = strdup(name);
                char* sep = strrchr(tmp, '/');
                if(sep) {
                    *sep='\0';
                    create_folder(tmp);
                    o = fopen(name, "wb");
                }
            }
            if (o==NULL) {
                printf("\tFailed\n");
            } else {
                fwrite(buff, sak->records[i].size, 1, o);
                fclose(o);
                printf("\tok\n");
            }
        }
    }
    fclose(f);
    free(buff);
}

void free_sak(sak_file *sak)
{
    if(!sak) return;
    for (int i=0; i<sak->size; i++)
    {
        free(sak->records[i].name);
    }
    free(sak->records);
    sak->size = sak->cap = 0;
    sak->records = NULL;
}

sak_file *copy_sak(sak_file *sak)
{
    if(!sak) return NULL;
    sak_file *cp = (sak_file*)malloc(sizeof(sak_file));
    memset(cp, 0, sizeof(sak_file));
    cp->sig = sak->sig;
    cp->ver = sak->ver;
    cp->cap = sak->cap*2;   // because we will probably add files...
    cp->records = (sak_record*)malloc(cp->cap*sizeof(sak_record));
    for (int i=0; i<sak->size; i++) {
        cp->records[i].name = strdup(sak->records[i].name);
        cp->records[i].size = sak->records[i].size;
        cp->records[i].offset = 0;  // to be calculated later
        cp->records[i].from_sak = sak->records[i].from_sak;
    }
    cp->size = sak->size;

    return cp;
}

/// Gives the size of the header once save on disk, include signature and version
int sak_header_size(sak_file* sak)
{
    if(!sak) return 0;
    int size = sizeof(uint32_t)*2+sizeof(uint16_t);
    for (int i=0; i<sak->size; i++) {
        size += strlen(sak->records[i].name) + 1 + sizeof(uint32_t);
    }
    return size;
}

void sak_reoffset(sak_file* sak)
{
    if(!sak) return;
    int off = sak_header_size(sak);
    for (int i=0; i<sak->size; i++) {
        sak->records[i].offset = off;
        off += sak->records[i].size;
    }
}

/// return -1 if not found, or index value if found
int is_insak(sak_file *sak, const char* name)
{
    if(!sak) return -1;
    for (int i=0; i<sak->size; i++)
        if(strcmp(sak->records[i].name, name)==0)
            return i;
    return -1;
}

void sak_addfile(sak_file* sak, const char* name, uint32_t size)
{
    if(!sak) return;
    int i = is_insak(sak, name);
    if(i==-1) i = sak->size++;  // append record, and increment size
    if(i == sak->cap) {
        sak->cap += 16;
        sak->records = (sak_record*)realloc(sak->records, sak->cap*sizeof(sak_record)); // bad, should test realloc worked
    }
    sak->records[i].name = strdup(name);
    sak->records[i].size = size;
    sak->records[i].offset = 0;
    sak->records[i].from_sak = 0;
}

void add_sak(const char* sakfile, sak_file *sak, const char* mask, const char* folder, const char* current)
{
    if(!sak) return;
    char name[4096];
    char next[4096];
    uint32_t size = 0;

    strcpy(name, folder);
    if(current) {
        strcat(name, "/");
        strcat(name, current);
    }
    DIR *d;
    struct dirent *dir;
    d = opendir(name);
    if (d)
    {
        while((dir = readdir(d)) != NULL)
        {
            if(dir->d_type==DT_DIR) {
                if((strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name, "..")!=0)) {
                    if(current) {
                        strcpy(next, current);
                        strcat(next, "/");
                    }
                    strcat(next, dir->d_name);
                    add_sak(sakfile, sak, mask, folder, next);
                }
            } else {
                // match with mask and add it if ok
                if(current) {
                    strcpy(next, current);
                    strcat(next, "/");
                    strcat(next, dir->d_name);
                } else {
                    strcpy(next, dir->d_name);
                }
                if(match(next, mask)) {
                    // ok try to add it
                    char myfile[4096];
                    strcpy(myfile, folder);
                    strcat(myfile, "/");
                    strcat(myfile, next);
                    FILE *f = fopen(myfile, "rb");
                    if (f) {
                        fseek(f, 0, SEEK_END);
                        uint32_t size = ftell(f);
                        fclose(f);
                        sak_addfile(sak, (char*)next, size);
                        printf("Adding \"%s\" as \"%s\" (size=%d)\n", myfile, next, size);
                    } else {
                        printf("Error openning \"%s\"\n", myfile);
                    }
                }
            }
        }
        closedir(d);
    }
}

void sak_writeheader(sak_file *sak, FILE *f)
{
    if(!sak) return;
    fwrite(&sak->sig, sizeof(sak->sig), 1, f);
    fwrite(&sak->ver, sizeof(sak->ver), 1, f);
    uint16_t us = sak->size;
    fwrite(&us, sizeof(us), 1, f);
    for (int i=0; i<us; i++) {
        fwrite(sak->records[i].name, strlen(sak->records[i].name)+1, 1, f);
        fwrite(&sak->records[i].offset, sizeof(uint32_t), 1, f);
    }
}

int main(int argc, char** argv)
{
    int cmd = -1;
    char* sakfile = NULL;
    char* folder = NULL;
    char* mask = NULL;

    printf("saktools v%d.%d\n", VER_MAJ, VER_MIN);

    if (argc<3 || (argc>1 && argv[1][0]=='h'))  {
        printf("\n\nUsage:\n\nsaktools CMD sakfile (folder) [mask]\n");
        printf("\twhere CMD is\n");
        printf("\t l : list content (no folder argument)\n");
        printf("\t e : extract file(s) (folder argument mandatory)\n");
        printf("\t a : add file(s) (folder argument mandatory)\n");
        return -1;
    }
    if(strcmp(argv[1],"l")==0) {
        cmd = 0; 
    }
    if(strcmp(argv[1],"e")==0) {
        cmd = 1; 
    }
    if(strcmp(argv[1],"x")==0) {
        cmd = 1; 
    }
    if(strcmp(argv[1],"a")==0) {
        cmd = 2; 
    }
    if(cmd<0) {
        printf("Unknown command '%s'\n", argv[1]);
        return -2;
    }
    sakfile = argv[2];
    if (argc>3) {
        if(cmd == 0)
            mask = argv[3];
        else
            folder = argv[3];
    }
    if (argc>4)
        mask = argv[4];
    
    switch(cmd) {
        case 0 : {
            sak_file* sak = read_sak(sakfile);

            list_sak_content(sak, mask);
            for (int j=4; j<argc; j++)
                list_sak_content(sak, argv[j]);

            free_sak(sak);
            free(sak);
        }
        break;
        case 1 : {
            if(!check_folder(folder)) {
                printf("ERROR: Bad folder name\n");
                return -5;
            }
            sak_file* sak = read_sak(sakfile);
            if(create_folder(folder)) {
                extract_sak(sakfile, sak, mask, folder);
                for (int j=5; j<argc; j++)
                    extract_sak(sakfile, sak, argv[j], folder);
            } else {
                printf("Error creating folder \"%s\"\n", folder);
            }

            free_sak(sak);
            free(sak);
        }
        break;
        case 2 : {
            if(!check_folder(folder)) {
                printf("ERROR: Bad folder name\n");
                return -5;
            }
            char newname[4096];
            strcpy((char*)newname, sakfile);
            strcat((char*)newname, ".XXXXXX");
            mkstemp(newname);
            sak_file* sak = read_sak(sakfile);
            sak_file* newsak = copy_sak(sak);
            add_sak(sakfile, newsak, mask, folder, NULL);
            for (int j=5; j<argc; j++)
                add_sak(sakfile, newsak, argv[j], folder, NULL);
            // ok, reoffset now
            sak_reoffset(newsak);
            FILE* f = fopen(newname, "wb");
            if (f) {
                sak_writeheader(newsak, f);
                void* buff = NULL;
                uint32_t size = 0;
                for(int i=0; i<newsak->size; i++) {
                    if(size<newsak->records[i].size) {
                        free(buff);
                        size = newsak->records[i].size;
                        buff = malloc(size);
                    }
                    if(newsak->records[i].from_sak) {
                        int j = is_insak(sak, newsak->records[i].name);
                        FILE *f2 = fopen(sakfile, "rb");
                        if(f2) {
                            fseek(f2, sak->records[j].offset, SEEK_SET);
                            fread(buff, newsak->records[i].size, 1, f2);
                            fclose(f2);
                        } else printf("WARNING, error while reading %s\n", newsak->records[i].name);
                    } else {
                        char name[4096];
                        strcpy(name, folder);
                        strcat(name, "/");
                        strcat(name, newsak->records[i].name);
                        FILE *f2 = fopen(name, "rb");
                        if(f2) {
                            fread(buff, newsak->records[i].size, 1, f2);
                            fclose(f2);
                        } else printf("WARNING, error while reading %s\n", newsak->records[i].name);
                    }
                    fwrite(buff, newsak->records[i].size, 1, f);
                }
            }
            fclose(f);
            // finish, remove old sak and replace with new one
            remove(sakfile);
            rename(newname, sakfile);
            // clean up memory
            free_sak(sak);
            free(sak);
            free_sak(newsak);
            free(newsak);
        }
        break;
    }

    return 0;
}