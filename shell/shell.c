#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys_api.h>

void getstring(char* string, uint32_t limit);
int get_argc(const char* string);
void get_argv(char** argv, char* string);

char prog_path[128];
char prog_stdin[128];
char prog_stdout[128];
char prog_stderr[128];
char prog_pwd[128];
char prog_name[128];
char prog_ser_name[128];

task_info_t prog_info = {
    prog_path,
    prog_stdin,
    prog_stdout,
    prog_stderr,
    prog_pwd,
    prog_name,
    prog_ser_name,
    0,
    0
};

int vdev;
int vdev_reg = 0;
uint8_t vdev_in;
int vdev_in_flag = 0;
uint8_t vdev_out;
int vdev_out_flag = 0;

// built in shell

int main(int argc, char** argv) {
    char input[256];
    int s_argc;
    char* s_argv[128];
    char pwd[2048];
    
    int c;
    uint64_t i;
    
    vfs_metadata_t metadata = {0};

    for (;;) {
        OS_pwd(pwd);
        printf("\e[32mechidnaOS\e[37m:\e[36m%s\e[37m# ", pwd);

        getstring(input, 256);
        s_argc = get_argc(input);
        get_argv(s_argv, input);

        if (!strcmp("heap", s_argv[0])) {
            printf("heap base: %d\n"
                   "heap size: %d\n", OS_get_heap_base(), OS_get_heap_size());
            OS_resize_heap(0x100);
        }

        else if (!strcmp("col", s_argv[0]))
            puts("\e[40m \e[41m \e[42m \e[43m \e[44m \e[45m \e[46m \e[47m \e[40m");
        
        else if (!strcmp("vdev", s_argv[0])) {
            if (vdev_reg) {
                puts("vdev already registered");
                continue;
            }
            puts("registering vdev");
            vdev = OS_vdev_register(&vdev_in, &vdev_in_flag, &vdev_out, &vdev_out_flag);
            if (vdev == -1) {
                puts("vdev registration failed");
                continue;
            }
            vdev_reg = 1;
            printf("vdev%d registered successfully\n", vdev);
            continue;
        }
        
        else if (!strcmp("vdevin", s_argv[0])) {
            if (!vdev_reg) {
                puts("vdev not registered");
                continue;
            }
            OS_vdev_in_ready(vdev);
            int vdevw = OS_vdev_await();
            printf("activity on vdev%d: `%c`\n", vdevw, (char)vdev_in);
            continue;
        }
        
        else if (!strcmp("vdevout", s_argv[0])) {
            if (s_argc == 1) continue;
            if (!vdev_reg) {
                puts("vdev not registered");
                continue;
            }
            vdev_out = (uint8_t)(*(s_argv[1]));
            OS_vdev_out_ready(vdev);
            int vdevw = OS_vdev_await();
            printf("activity on vdev%d\n", vdevw);
            continue;
        }
        
        else if (!strcmp("div0", s_argv[0])) {/*
            int a = 0;
            a = a / 0;
*/        }
        
        else if (!strcmp("esc", s_argv[0])) {
            if (s_argc == 1) continue;
            putchar('\e');
            fputs(s_argv[1], stdout);
        }
        
        else if (!strcmp("mkdir", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_mkdir(s_argv[1], 0))
                fprintf(stderr, "couldn't create directory `%s`.\n", s_argv[1]);
        }
        
        else if (!strcmp("fork", s_argv[0])) {
            
            pid_t pid = fork();

            if (pid == -1) {
                puts("Fork fail.");
            }
            else if (pid == 0) {
                puts("Child process OK.");
            }
            else {
                printf("Child PID: %d\n", pid);
            }

        }
        
        else if (!strcmp("ls", s_argv[0])) {
            char* ls_path;
            if (s_argc == 1) ls_path = pwd;
            else ls_path = s_argv[1];
            for (int i = 0; ; i++) {
                if (OS_vfs_list(ls_path, &metadata, i) == -2) break;
                if (metadata.filetype == 1) fputs("\e[36m", stdout);
                if (metadata.filetype == 2) fputs("\e[33m", stdout);
                printf("%s", metadata.filename);
                fputs("\e[37m", stdout);
                putchar('\n');
            }
        }
        
        else if (!strcmp("beep", s_argv[0])) {
            if (s_argc == 1) continue;
            OS_vfs_write("/dev/pcspk", 0, atoi(s_argv[1]));
        }
        
        else if (!strcmp("rdspk", s_argv[0]))
            printf("%d\n", OS_vfs_read("/dev/pcspk", 0));
        
        else if (!strcmp("send", s_argv[0])) {
            char server[] = "server";
            uint32_t pid = OS_ipc_resolve_name(server);
            printf("%s's PID is: %d\n", server, pid);
            printf("payload is: %s\n", s_argv[1]);
            OS_ipc_send_packet(pid, s_argv[1], strlen(s_argv[1]) + 1);
        }
        
        else if (!strcmp("exit", s_argv[0])) return 0;
        
        else if (!strcmp("dump", s_argv[0])) {
            FILE* fp;
            if (s_argc == 1) continue;
            if (!(fp = fopen(s_argv[1], "r"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[1]);
                continue;
            }
            while ((c = fgetc(fp)) != EOF)
                putchar(c);
            printf("\nFile length: %d\n", (int)ftell(fp));
            fclose(fp);
        }
        
        else if (!strcmp("dumpr", s_argv[0])) {
            FILE* src;
            FILE* dest;
            if (s_argc < 3) continue;
            if (!(src = fopen(s_argv[1], "rb"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[1]);
                continue;
            }
            if (!(dest = fopen(s_argv[2], "rb+"))) {
                fprintf(stderr, "can't open `%s`.\n", s_argv[2]);
                continue;
            }
            while ((c = fgetc(src)) != EOF)
                fputc(c, dest);
            continue;
        }
        
        else if (!strcmp("cd", s_argv[0])) {
            if (s_argc == 1) continue;
            if (OS_vfs_cd(s_argv[1]) == -2)
                printf("shell: invalid directory: `%s`.\n", s_argv[1]);
        }

        // return to prompt if no input
        else if (!input[0]) continue;

        // if the input did not match any command
        else {
            strcpy(prog_path, s_argv[0]);
            strcpy(prog_name, s_argv[0]);
            OS_what_stdin(prog_stdin);
            OS_what_stdout(prog_stdout);
            OS_what_stderr(prog_stderr);
            OS_pwd(prog_pwd);
            prog_info.argc = s_argc;
            prog_info.argv = s_argv;
            *prog_ser_name = 0;
            if (OS_general_execute_block(&prog_info) == -1)
                printf("shell: invalid command: `%s`.\n", input);
        }
    }
    
    return 0;
}

int get_argc(const char* string) {
    uint32_t index=0;
    int argc=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            index++;
            continue;
        }
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        argc++;
    }
    return argc;
}

void get_argv(char** argv, char* string) {
    uint32_t index=0;
    uint8_t arg=0;
    
    while (string[index]) {
        if (string[index] == ' ') {
            string[index++] = 0;
            continue;
        }
        argv[arg++] = &string[index];
        while ((string[index]!=' ') && (string[index]!='\0'))
            index++;
        string[index++] = 0;
    }
    return;
}

void getstring(char* string, uint32_t limit) {
    uint32_t x=0;
    int c;
    while (1) {
        c = getchar();
        if (c=='\b') {
            if (x) {
                x--;
            }
        } else if (c=='\n') break;
        else if (x<(limit-1)) {
            string[x] = c;
            x++;
        }
    }
    string[x] = 0x00;
    return;
}
