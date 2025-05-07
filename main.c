#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_OPERATII 100
#define MAX_PROGRAMARI 500
#define FISIER_PROGRAMARI "Programari.txt"
#define FISIER_OFERTE "Oferte.txt"

#ifdef _WIN32
    #define CLEAR "cls"
#else
    #define CLEAR "clear"
#endif

typedef struct {
    char nume[50];
    float pret;
    int durata;
} Operatie;

typedef struct {
    char data[15];
    char start[6];
    char end[6];
    char nume_pacient[50];
    char operatie[50];
    int start_min;
} DisponibilitateEntry;

Operatie operatii[MAX_OPERATII];
int numar_operatii = 0;

int valid_nume(char s[])
{
    int len = strlen(s);

    while (len > 0 && (s[len-1] == '\n' || s[len-1] == ' ')) {
        s[--len] = '\0';
    }
    if (len == 0) return 0;

    int space_index = -1;
    for (int i = 0; i < len; i++) {
        if (s[i] == ' ') {
            if (space_index != -1) return 0;
            space_index = i;
        }
    }

    if (space_index <= 0 || space_index >= len - 1) return 0;

    if (!isupper(s[0])) return 0;
    for (int i = 1; i < space_index; i++) {
        if (!islower(s[i])) return 0;
    }

    int start2 = space_index + 1;
    if (!isupper(s[start2])) return 0;
    for (int i = start2 + 1; i < len; i++) {
        if (!islower(s[i])) return 0;
    }

    return 1;
}



int valid_data(int zi, int luna, int an) {
    if (an < 2000 || luna < 1 || luna > 12 || zi < 1) return 0;
    int zile_luna[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if ((an%400==0) || (an%4==0 && an%100!=0))
        zile_luna[2]=29;
    if (zi > zile_luna[luna])
        return 0;
    return 1;
}

int valid_ora(int h, int m) {
    return (h>=8 && h<17 && m>=0 && m<60);
}


void citesteOferte() {
    FILE *f = fopen(FISIER_OFERTE, "r");
    if (!f) {
        printf("Eroare la deschiderea fisierului Oferte.txt. Se vor folosi ofertele implicite.\n");
        return;
    }
    while (numar_operatii < MAX_OPERATII && fscanf(f, "%49s %f %d", operatii[numar_operatii].nume,&operatii[numar_operatii].pret,&operatii[numar_operatii].durata) == 3)
    {
            numar_operatii++;
    }
    fclose(f);
}

void afiseazaOferta() {
    system(CLEAR);
    printf("---\033[94m Oferte de operatii \033[0m---\n");

    Operatie op;
    int j=1;
    for (int i=0;i<numar_operatii;i++) {
        printf("%d. \033[95m%s\033[0m - \033[32m%.2f RON\033[0m - \033[33m%d minute\033[0m\n", j++, operatii[i].nume, operatii[i].pret, operatii[i].durata);
    }

    printf("\nApasati Enter pentru a reveni la meniu...");
    getchar();
    system(CLEAR);
}

int compareEntries(const void *a, const void *b) {
    return ((DisponibilitateEntry*)a)->start_min - ((DisponibilitateEntry*)b)->start_min;
}

void Disponibilitate() {
    system(CLEAR);
    printf("---\033[94m Verificare disponibilitate \033[0m---\n");
    char buffer[64];
    int zi,luna,an;
    while (1) {
        printf("Introduceti \033[33mdata (zz-ll-aaaa)\033[0m sau Enter pentru meniu: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (buffer[0]=='\n') { system(CLEAR); return; }
        if (sscanf(buffer, "%d-%d-%d", &zi, &luna, &an)==3 && valid_data(zi,luna,an))
            break;
        printf("Data invalida. Format corect: zz-ll-aaaa si date reale.\n");
    }
    char zi_data[15];
    sprintf(zi_data, "%02d-%02d-%04d", zi, luna, an);

    DisponibilitateEntry list[MAX_PROGRAMARI];
    int count=0;
    FILE *f = fopen(FISIER_PROGRAMARI, "r");
    if (f) {
        char line[200];
        while (fgets(line, sizeof(line), f)) {
            char data[15], timestr[20], nume[50], opn[50];
            if (sscanf(line, "%14s %19s %49s %49s", data, timestr, nume, opn)==4 && strcmp(data, zi_data)==0) {
                char *dash = strchr(timestr, '-');
                if (dash) {
                    *dash='\0'; char *start_s=timestr, *end_s=dash+1;
                    int h,m;
                    if (sscanf(start_s, "%d:%d", &h, &m)==2)
                        list[count].start_min = h*60+m;
                    else list[count].start_min=0;

                    snprintf(list[count].start, 6, "%s", start_s);
                    snprintf(list[count].end, 6, "%s", end_s);
                    strcpy(list[count].data, data);
                    strcpy(list[count].nume_pacient, nume);
                    strcpy(list[count].operatie, opn);
                    count++;
                }
            }
        }
        fclose(f);
    }

    if (count==0) printf("Nu exista programari pentru data %s.\n", zi_data);
    else {
        qsort(list, count, sizeof(DisponibilitateEntry), compareEntries);
        printf("\nProgramari pentru %s:\n", zi_data);
        for (int i=0; i<count; i++)
            printf("%d. %s-%s  %s  %s\n", i+1, list[i].start, list[i].end, list[i].nume_pacient, list[i].operatie);
    }

    int raspuns;
    while (1) {
        printf("\nDoriti sa va programati in aceasta zi?\n");
        printf("1.Da\n");
        printf("2.Nu\n");
        fgets(buffer, sizeof(buffer), stdin);
        if (buffer[0]=='\n') {
            system(CLEAR);
            return;
        }
        if (sscanf(buffer, "%d", &raspuns)==1 && (raspuns==1 || raspuns==2)) break;
        printf("Optiune invalida. Introduceti 1 sau 2.\n");
    }
    if (raspuns!=1) { system(CLEAR); return; }

    char nume_p[50], op_name[50];
    while (1) {
        printf("Introduceti \033[33mnumele pacientului\033[0m (Nume Prenume): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (buffer[0]=='\n') {
            system(CLEAR);
            return;
        }
        else {
            if (valid_nume(buffer)) {
                break;
            }
            else {
                printf("Numele este invalid. (Nume Prenume)\n");
            }
        }
    }
    strcpy(nume_p, strtok(buffer, "\n"));

    int durata=0, valid_op=0;
    while (!valid_op) {
        printf("Introduceti \033[33mdenumirea operatiei\033[0m: ");
        fgets(buffer, sizeof(buffer), stdin);
        if (buffer[0]=='\n') { system(CLEAR); return; }
        strcpy(op_name, strtok(buffer, "\n"));
        for (int i=0; i<numar_operatii; i++) {
            if (strcmp(operatii[i].nume, op_name)==0) {
                durata = operatii[i].durata; valid_op=1; break;
            }
        }
        if (!valid_op) printf("Operatie invalida. Incercati din nou.\n");
    }

    char timpul[6]; int start_min, end_min;
    while (1) {
        printf("Introduceti \033[33mora dorita (hh:mm)\033[0m in intervalul 08:00-17:00 : ");
        fgets(buffer, sizeof(buffer), stdin);
        if (buffer[0]=='\n') { system(CLEAR); return; }
        int h,m;
        if (sscanf(buffer, "%2d:%2d", &h, &m)==2 && valid_ora(h,m)) {
            start_min=h*60+m;
            end_min=start_min+durata;
            int ocupat=0;
            for (int i=0; i<count; i++) {
                int ex_h, ex_m;
                sscanf(list[i].end, "%d:%d", &ex_h, &ex_m);
                int ex_start=list[i].start_min;
                int ex_end=ex_h*60+ex_m;
                if (!(end_min<=ex_start || start_min>=ex_end)) { ocupat=1; break; }
            }
            if (!ocupat) { snprintf(timpul, 6, "%02d:%02d", h, m); break; }
            printf("Interval %02d:%02d-%02d:%02d ocupat.\n", h, m, end_min/60, end_min%60);
        } else {
            printf("Format ora invalid. hh:mm in interval 08:00-17:00.\n");
        }
    }

    FILE *fw = fopen(FISIER_PROGRAMARI, "a");
    fprintf(fw, "%s %s-%02d:%02d %s %s\n",
            zi_data, timpul, end_min/60, end_min%60, nume_p, op_name);
    fclose(fw);
    printf("Programare adaugata: %s %s-%02d:%02d %s %s\n",
           zi_data, timpul, end_min/60, end_min%60, nume_p, op_name);
    printf("\nApasati Enter pentru a reveni la meniu...");
    getchar();
    system(CLEAR);
}

void afiseazaIstoric() {
    system(CLEAR);
    printf("---\033[94m Vizualizare istoric pacient \033[0m---\n");
    char buffer[100];

    while (1) {
        printf("Introduceti \033[33mnumele pacientului\033[0m (Nume Prenume): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (buffer[0]=='\n') {
            system(CLEAR);
            return;
        }
        else {
            if (valid_nume(buffer)) {
                break;
            }
            else {
                printf("Numele este invalid. (Nume Prenume)\n");
            }
        }
    }
    char targetName[50];
    strncpy(targetName, buffer, sizeof(targetName));
    targetName[sizeof(targetName)-1] = '\0';

    FILE *f = fopen(FISIER_PROGRAMARI, "r");
    if (!f) {
        printf("Fisier programari inexistent. \n");
        printf("\nApasati Enter pentru a reveni la meniu...");
        getchar();
        system(CLEAR);
        return;
    }

    char line[512];
    int gasit = 0;
    printf("\nIstoric programari pentru \"%s\":\n\n", targetName);

    while (fgets(line, sizeof(line), f)) {
        char data[15], timestr[20], rest[200];
        if (sscanf(line, "%14s %19s %[^\n]", data, timestr, rest) != 3)
            continue;


        char *lastSp = strrchr(rest, ' ');
        if (!lastSp)
            continue;

        *lastSp = '\0';
        char *name = rest;
        char *operation = lastSp+1;

        if (strcmp(name, targetName) == 0) {
            printf("\033[31m%s\033[0m " "\033[32m%s\033[0m \033[33m%s\033[0m \033[34m%s\033[0m\n", data, timestr, name, operation
);
            gasit = 1;
        }
    }
    fclose(f);

    if (!gasit)
        printf("Nicio programare gasita pentru %s.\n", targetName);

    printf("\nApasati Enter pentru a reveni la meniu...");
    getchar();
    system(CLEAR);
}


int main() {
    int optiune;
    citesteOferte();
    do {
        printf("---\033[94m Cabinet Stomatologic \033[0m---\n");
        printf("1. Vizualizare oferta operatii\n");
        printf("2. Verificare disponibilitate\n");
        printf("3. Vizualizare istoric programari\n");
        printf("4. Iesire\n");
        printf("\033[32mAlegeti o optiune: \033[0m");
        scanf("%d", &optiune);
        getchar();
        switch (optiune) {
            case 1:
                {
                    afiseazaOferta();
                    break;
                }
            case 2:
                {
                    Disponibilitate();
                    break;
                }
            case 3:
                {
                    afiseazaIstoric();
                    break;
                }
            case 4:
                {
                    printf("Iesire...\n");
                    break;
                }
            default:
                printf("Optiune invalida!\n");
                printf("\nApasati Enter pentru a introduce un alt numar...");
                getchar(); system(CLEAR);
        }
    } while (optiune != 4);
    return 0;
}
