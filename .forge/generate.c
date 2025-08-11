
int ensure_dir(const char *p) {
    if(mkdir((p), 0755) == 0) return 0;
    if(errno == EEXIST) return 0;
    return -1;
}

__attribute__((visibility("default")))
void generate_project(const char *project_name) {
    // generate_project
    printf("%s\n", project_name);
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
        "clang -E -P -x c "
        "-DINIT_FILE=\\\"%s\\\" "
        ".forge/templates/launcher.c -o '%s/src/launcher.c'",
        ".forge/templates/glfw/init.c", project_name);
    system(cmd);
}