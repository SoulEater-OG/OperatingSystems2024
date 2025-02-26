#ifndef PARSER_H
#define PARSER_H

void parse_command(char *command, char **args);
void split_commands(char *input, char ***commands, int *num_commands);
void bg_command(char **args);
void fg_command(char **args);
void jobs_command();
void kill_command(char **args);
void split_pipes(char *command, char ***cmds, int *num_cmds);

#endif // PARSER_H
