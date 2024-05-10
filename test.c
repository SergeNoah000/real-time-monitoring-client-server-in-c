#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

char* searchFile(const char *folder, const char *filename) {
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    static char filepath[1024];

    if ((dir = opendir(folder)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        sprintf(filepath, "%s/%s", folder, entry->d_name);
        if (stat(filepath, &filestat) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(filestat.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char *subfolder = (char*)malloc(strlen(filepath) + 1);
            strcpy(subfolder, filepath);
            char *found = searchFile(subfolder, filename);
            free(subfolder);
            if (found != NULL) {
                closedir(dir);
                return found;
            }
        } else if (S_ISREG(filestat.st_mode) && strcmp(entry->d_name, filename) == 0) {
            closedir(dir);
            return filepath;
        }
    }

    closedir(dir);
    return NULL;
}

int main() {
    const char *folder = "data"; // Dossier où rechercher
    const char *filename = "je-suis-du-niveau-2.bin"; // Nom du fichier à rechercher

    char *result = searchFile(folder, filename);
    if (result != NULL)
        printf("Chemin relatif du fichier trouvé : %s\n", result);
    else
        printf("Le fichier n'a pas été trouvé.\n");

    return 0;
}
