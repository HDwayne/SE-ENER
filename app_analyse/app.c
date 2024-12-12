#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DEFAULT_FREQUENCY 10
#define OUTPUT_FOLDER "./data/"
#define MOJITOS "../mojitos/bin"
#define MAX_LINE_LEN 1024
#define JSON_FILE "./data/results.json"

typedef struct {
    char *command;
    char **params;
    int param_count;
    double time;
    double energy;
    double power;
} Result;

Result *results = NULL;

//prototype
static void usage(const char *progname);
static void check_root(void);
static void ensure_output_folder(const char *folder);
static pid_t run_mojitos(int frequency, const char *output_file_path);
static pid_t run_command(int argc, char *argv[], int cmd_index);
static void wait_for_command(pid_t cmd_pid);
static void stop_mojitos(pid_t mojitos_pid);
static void compute_and_print_stats(const char *output_file_path);
static void append_to_json_file(const char *command, char **params, int param_count, double time, double energy, double power);


static void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-f frequency] <command_to_monitor> [args...]\n", progname);
    exit(EXIT_FAILURE);
}

static void check_root(void) {
    if (geteuid() != 0) {
        fprintf(stderr, "Please run as root (with sudo)\n");
        exit(EXIT_FAILURE);
    }
}

static void ensure_output_folder(const char *folder) {
    struct stat st = {0};
    if (stat(folder, &st) == -1) {
        if (mkdir(folder, 0777) != 0) {
            perror("mkdir OUTPUT_FOLDER");
            exit(EXIT_FAILURE);
        }
        printf("Created directory %s\n", folder);
    }
}

static pid_t run_mojitos(int frequency, const char *output_file_path) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        char freq_arg[64];
        snprintf(freq_arg, sizeof(freq_arg), "%d", frequency);
        execl(MOJITOS "/mojitos", "mojitos", "-f", freq_arg, "-o", output_file_path, "-r", (char *)NULL);
        perror("execl mojitos");
        exit(EXIT_FAILURE);
    }

    printf("MojitO/S started with PID %d\n", pid);
    return pid;
}

static pid_t run_command(int argc, char *argv[], int cmd_index) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull == -1) {
            perror("open /dev/null");
            exit(EXIT_FAILURE);
        }
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        close(devnull);

        execvp(argv[cmd_index], &argv[cmd_index]);
        perror("execvp monitored command");
        exit(EXIT_FAILURE);
    }

    printf("Monitoring command '%s' with PID %d\n", argv[cmd_index], pid);
    return pid;
}

static void wait_for_command(pid_t cmd_pid) {
    int status;
    if (waitpid(cmd_pid, &status, 0) == -1) {
        perror("waitpid");
    } else {
        printf("Monitored command finished.\n");
    }
}

static void stop_mojitos(pid_t mojitos_pid) {
    printf("Stopping MojitO/S...\n");
    kill(mojitos_pid, SIGTERM);
    waitpid(mojitos_pid, NULL, 0);
}

/**
 * Fonction pour calculer et afficher les stats.
 * On affiche :
 * - Le temps total (tt)
 * - L'énergie cumulée par colonne (en Joules)
 * - L'énergie cumulée totale (toutes colonnes d'énergie confondues)
 * - La puissance moyenne (W)
 */
static void compute_and_print_stats(const char *output_file_path) {
    printf("Data collection complete. File saved to %s\n", output_file_path);

    if (access(output_file_path, F_OK) != 0) {
        fprintf(stderr, "Error: %s not found.\n", output_file_path);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(output_file_path, "r");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char header_line[MAX_LINE_LEN];
    if (!fgets(header_line, sizeof(header_line), fp)) {
        fprintf(stderr, "Error reading header line from %s\n", output_file_path);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    char *headers_copy = strdup(header_line);
    if (!headers_copy) {
        perror("strdup");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    char *newline = strchr(headers_copy, '\n');
    if (newline) *newline = '\0';

    // On tokenize par espaces
    int col_count = 0;
    char *token = strtok(headers_copy, " \t");
    char **colnames = NULL;
    while (token) {
        col_count++;
        colnames = realloc(colnames, col_count * sizeof(char *));
        if (!colnames) {
            perror("realloc");
            free(headers_copy);
            fclose(fp);
            exit(EXIT_FAILURE);
        }
        colnames[col_count - 1] = strdup(token);
        token = strtok(NULL, " \t");
    }

    if (col_count < 2) {
        fprintf(stderr, "Not enough columns in data file.\n");
        for (int i = 0; i < col_count; i++) free(colnames[i]);
        free(colnames);
        free(headers_copy);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    // Tableau pour stocker l'énergie cumulée par colonne
    double *energy_sum_uJ = calloc(col_count, sizeof(double));
    if (!energy_sum_uJ) {
        perror("calloc");
        for (int i = 0; i < col_count; i++) free(colnames[i]);
        free(colnames);
        free(headers_copy);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    double start_time = 0.0;
    double end_time = 0.0;
    int first_data_line = 1;

    char line[MAX_LINE_LEN];
    int data_count = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *line_copy = strdup(line);
        if (!line_copy) {
            perror("strdup");
            break;
        }

        newline = strchr(line_copy, '\n');
        if (newline) *newline = '\0';

        double values[col_count];
        int field_idx = 0;
        char *field = strtok(line_copy, " \t");
        while (field && field_idx < col_count) {
            values[field_idx] = atof(field);
            field = strtok(NULL, " \t");
            field_idx++;
        }
        free(line_copy);

        if (field_idx < col_count) {
            // Ligne incomplète, on ignore
            continue;
        }

        double timestamp = values[0];

        if (first_data_line) {
            start_time = timestamp;
            // Ajout des énergies pour cette ligne
            for (int i = 1; i < col_count; i++) {
                energy_sum_uJ[i] += values[i];
            }
            first_data_line = 0;
            data_count = 1;
        } else {
            end_time = timestamp;
            for (int i = 1; i < col_count; i++) {
                energy_sum_uJ[i] += values[i];
            }
            data_count++;
        }
    }

    fclose(fp);

    double tt = end_time - start_time; // Temps total en s

    // Calcul de l'énergie totale (somme de toutes les colonnes d'énergie)
    double total_energy_uJ = 0.0;
    for (int i = 1; i < col_count; i++) {
        total_energy_uJ += energy_sum_uJ[i];
    }

    double total_energy_J = total_energy_uJ / 1e6;
    double puissance = 0.0;
    if (tt > 0) {
        puissance = total_energy_J / tt;
    }

    printf("\nRésultats:\n");
    printf("Temps total (s): %.6f\n", tt);
    printf("Énergie totale (J): %.6f\n", total_energy_J);
    printf("Puissance moyenne (W): %.6f\n\n", puissance);

    printf("Détail par colonne:\n");
    // Affiche la liste des colonnes et leur énergie totale en Joules
    // La première colonne est le timestamp, donc on commence à i=1
    for (int i = 1; i < col_count; i++) {
        double column_energy_J = energy_sum_uJ[i] / 1e6;
        printf("%s: %.6f J\n", colnames[i], column_energy_J);
    }

    results->time = tt;
    results->energy = total_energy_J;
    results->power = puissance;

    // Nettoyage
    free(energy_sum_uJ);
    for (int i = 0; i < col_count; i++)
        free(colnames[i]);
    free(colnames);
    free(headers_copy);
}

static void append_to_json_file(const char *command, char **params, int param_count, double time, double energy, double power) {
    FILE *fp = fopen(JSON_FILE, "r+");
    if (!fp) {
        // If the file doesn't exist, create it and start a new JSON array
        fp = fopen(JSON_FILE, "w");
        if (!fp) {
            perror("fopen JSON_FILE");
            return;
        }
        fprintf(fp, "[\n");
    } else {
        // Check if the file is empty
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        if (file_size == 0) {
            fprintf(fp, "[\n");
        } else {
            // Move back to overwrite the closing bracket
            fseek(fp, -2, SEEK_END);
            fprintf(fp, ",\n");
        }
    }

    // Write the JSON entry
    fprintf(fp, "  {\n");
    fprintf(fp, "    \"command\": \"%s\",\n", command);
    fprintf(fp, "    \"params\": [");
    for (int i = 0; i < param_count; i++) {
        fprintf(fp, "\"%s\"%s", params[i], (i < param_count - 1) ? ", " : "");
    }
    fprintf(fp, "],\n");
    fprintf(fp, "    \"time\": %.2f,\n", time);
    fprintf(fp, "    \"energy\": %.2f,\n", energy);
    fprintf(fp, "    \"power\": %.2f\n", power);
    fprintf(fp, "  }\n");
    fprintf(fp, "]\n");

    fclose(fp);
    printf("Results appended to %s\n", JSON_FILE);
}

int main(int argc, char *argv[]) {
    int frequency = DEFAULT_FREQUENCY;
    int json_output = 0;

    if (argc < 2) {
        usage(argv[0]);
    }

    int i = 1;
    // Parse les options de l'appli (-f)
    while (i < argc) {
        if (strcmp(argv[i], "-f") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Missing value after -f\n");
                usage(argv[0]);
            }
            frequency = atoi(argv[i]);
            if (frequency <= 0) {
                fprintf(stderr, "Invalid frequency.\n");
                exit(EXIT_FAILURE);
            }
            i++;
        } else if (strcmp(argv[i], "--json") == 0) {
            json_output = 1;
            i++;
        } else {
            // On est arrivé à un argument non-option
            break;
        }
    }

    // A ce stade, i pointe sur la commande monitorée (ou la fin des arguments)
    if (i >= argc) {
        fprintf(stderr, "No command specified.\n");
        usage(argv[0]);
    }

    int cmd_index = i;

    results = malloc(sizeof(Result));
    if (!results) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    results->command = argv[cmd_index];
    results->params = &argv[cmd_index + 1];
    results->param_count = argc - cmd_index - 1;
    
    check_root();
    ensure_output_folder(OUTPUT_FOLDER);

    char output_file_path[1024];
    snprintf(output_file_path, sizeof(output_file_path), "%sdata.csv", OUTPUT_FOLDER);
    unlink(output_file_path);

    pid_t mojitos_pid = run_mojitos(frequency, output_file_path);
    pid_t cmd_pid = run_command(argc, argv, cmd_index);
    wait_for_command(cmd_pid);
    stop_mojitos(mojitos_pid);

    compute_and_print_stats(output_file_path);

    if (json_output) {
        append_to_json_file(results->command, results->params, results->param_count, results->time, results->energy, results->power);
    }

    return 0;
}