// Copyright 2023 Dominik Marcinowski <dmarcinowski@zoho.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef TINI_H_
#define TINI_H_

#include <stddef.h>

#ifndef TINIDEF
#define TINIDEF static inline
#endif

#define TINI_GLOBAL_SEC "global"
#define TINI_SECTION_LIMIT 255

typedef struct {
    char *name;
    char *value;
} TiniEntry;

typedef struct {
    char *name;
    size_t entr_sz;
    TiniEntry *entr;
} TiniSection;

typedef struct {
    size_t sect_sz;
    TiniSection *sect;
} TiniFile;

TINIDEF void tini_free(TiniFile *file);
TINIDEF TiniFile *tini_read(const char *path);
TINIDEF TiniEntry *tini_get_entry(TiniSection *sec, const char *name);
TINIDEF TiniSection *tini_get_section(TiniFile *file, const char *name);

#endif // TINI_H_

#ifdef TINI_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TINIDEF void tini_free(TiniFile *file)
{
    for (size_t i = 0; i < file->sect_sz; ++i) {
        TiniSection *sect = &file->sect[i];
        free(sect->name);
        for (size_t j = 0; j < sect->entr_sz; ++j) {
            TiniEntry *entr = &sect->entr[j];
            free(entr->name);
            free(entr->value);
        }
        free(sect->entr);
    }

    free(file->sect);
    free(file);
}

TINIDEF TiniFile *tini_read(const char *path)
{
    TiniFile *tini_file = malloc(sizeof(TiniFile));
    tini_file->sect = NULL;
    tini_file->sect_sz = 0;

    FILE *file = fopen(path, "r");
    if (file != NULL) {
        char line[256];

        while (fgets(line, 256, file)) {
            if ((strstr(line, ";") != NULL) || (strstr(line, "#") != NULL)) {
                for (size_t i = 0; i < strlen(line) - 1; ++i) {
                    if ((line[i] == ';') || (line[i] == '#')) {
                        line[i] = '\0';
                    }
                }
            }

            // Parse and add section to file
            if (strncmp(line, "[", 1) == 0) {
                char name[255] = {0};
                for (size_t i = 1; i < strlen(line) - 1; ++i) {
                    if (line[i] == ']') break;
                    name[i-1] = line[i];
                }

                TiniSection sect = {
                    .name = malloc(strlen(name)+1),
                    .entr = NULL,
                    .entr_sz = 0
                };
                strcpy(sect.name, name);

                tini_file->sect = realloc(tini_file->sect, (tini_file->sect_sz + 1) * sizeof(TiniSection));
                tini_file->sect[tini_file->sect_sz] = sect;
                tini_file->sect_sz++;

                continue;
            }

            if (strstr(line, "=") != NULL) {
                char name[255] = {0};
                char value[256] = {0};

                int eq = -1, n = 0, v = 0;
                for (size_t i = 0; i < strlen(line); ++i) {
                    if (eq == -1 && line[i] == '=') {
                        eq = i;
                        while (line[i+1] == ' ') i++;
                        continue;
                    }
                    if (eq == -1) {
                        if (line[i] == ' ') continue;
                        name[n] = line[i];
                        n++;
                    } else {
                        if (line[i] == '\r') break;
                        if (line[i] == '\n') break;
                        value[v] = line[i];
                        v++;
                    }
                }
                for (size_t i = strlen(value)-1; i > 0; --i) {
                    if (value[i] != ' ') break;
                    value[i] = '\0';
                }

                TiniEntry entr = {
                    .name = malloc(strlen(name)+1),
                    .value = malloc(strlen(value)+1)
                };
                strcpy(entr.name, name);
                strcpy(entr.value, value);

                tini_file->sect->entr = realloc(tini_file->sect->entr, (tini_file->sect->entr_sz + 1) * sizeof(TiniEntry));
                tini_file->sect->entr[tini_file->sect->entr_sz] = entr;
                tini_file->sect->entr_sz++;

                continue;
            }
        }
        fclose(file);
    } else {
        return NULL;
    }

    return tini_file;
}

TINIDEF TiniEntry *tini_get_entry(TiniSection *sect, const char *name)
{
    for (size_t i = 0; i < sect->entr_sz; ++i) {
        TiniEntry *entr = &sect->entr[i];
        if (strcmp(sect->name, name) == 0) {
            return entr;
        }
    }
    return NULL;
}

TINIDEF TiniSection *tini_get_section(TiniFile *file, const char *name)
{
    for (size_t i = 0; i < file->sect_sz; ++i) {
        TiniSection *sect = &file->sect[i];
        if (strcmp(sect->name, name) == 0) {
            return sect;
        }
    }
    return NULL;
}

#endif // TINI_IMPLEMENTATION
