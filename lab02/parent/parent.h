#ifndef LAB02_PARENT_H
#define LAB02_PARENT_H

void print_envp(char** envp, int len);
int get_len_envp(char** envp);
char** add_variable_to_envp(char** envp);
char** create_child_env(char* file_name);
char* get_child_path(char** env);

#endif //LAB02_PARENT_H
