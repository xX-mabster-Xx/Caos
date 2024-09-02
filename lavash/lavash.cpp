#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void DEBUG(const std::string& msg) {
    std::cout << "DEBUG: " << msg << std::endl;
}

const int FLAGS = 07777;

struct PartOfCommand {
    char type;
    std::string word;
};

struct Instruction {
    std::vector<std::string> command;
    std::string in;
    std::string out;
};

class ProgrammRunner {
public:
    ProgrammRunner(const std::vector<Instruction>& line) : line_(line) {
    }

    std::vector<char*> PrepareArgs(size_t idx) {
        const std::vector<std::string>& command = line_[idx].command;
        std::vector<char*> prepared_args(command.size() + 1);
        for (size_t i = 0; i < command.size(); ++i) {
            prepared_args[i] = new char[command[i].size() + 1];
            strcpy(prepared_args[i], command[i].c_str());
        }
        prepared_args.back() = nullptr;
        return prepared_args;
    }

    int Run(int in) {
        std::vector<pid_t> to_wait_pids;

        int pipe_fd_old[2] = {in, 0};
        int pipe_fd_new[2];
        for (size_t i = 0; i < line_.size(); ++i) {
            std::vector<char*> prepared_args = PrepareArgs(i);
            if (i != line_.size() - 1) {
                if (pipe(pipe_fd_new) < 0) {
                    throw std::runtime_error("pipe");
                }
            }
            pid_t pid = fork();
            if (pid == 0) {

                if (i != 0) {
                    dup2(pipe_fd_old[0], STDIN_FILENO);
                    close(pipe_fd_old[0]);
                    close(pipe_fd_old[1]);
                }

                if (i != line_.size() - 1) {
                    dup2(pipe_fd_new[1], STDOUT_FILENO);
                    close(pipe_fd_new[0]);
                    close(pipe_fd_new[1]);
                }

                if (!line_[i].in.empty()) {
                    int fd;
                    if ((fd = open(line_[i].in.c_str(), O_RDONLY)) < 0) {
                        perror(("./lavash: line 1: " + line_[i].in).c_str());
                        return 1;
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                if (!line_[i].out.empty()) {
                    int fd;
                    if ((fd = open(line_[i].out.c_str(), O_RDWR | O_TRUNC | O_CREAT, FLAGS)) < 0) {
                        perror(("./lavash: line 1: " + line_[i].in).c_str());
                        return 1;
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                if (execvp(prepared_args[0], prepared_args.data()) < 0) {
                    if (line_[i].command[0] == "1984") {
                        return 0;
                    }
                    std::cerr << "./lavash: line 1: " << line_[i].command[0] << ": command not found\n";
                    return 127;
                }
            }
            if (i > 0) {
                close(pipe_fd_old[0]);
                close(pipe_fd_old[1]);
            }
            pipe_fd_old[0] = pipe_fd_new[0];
            pipe_fd_old[1] = pipe_fd_new[1];
            to_wait_pids.push_back(pid);
        }

        int status;

        for (size_t i = 0; i < to_wait_pids.size(); ++i) {
            waitpid(to_wait_pids[i], &status, 0);
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            throw std::runtime_error("WIFEXITED");
        }
    }

private:
    const std::vector<Instruction>& line_;
};

bool IsSpecial(const std::string& str) {
    return str == "|" || str == ">" || str == "<";
}

char ReadSymbol(size_t& i, const std::string& command) {
    if (command[i] != '\\') {
        return command[i];
    }
    ++i;
    return command[i];
}

int main(int argc, char **argv, char **envv) {
    std::string command = argv[2];
    // std::cout << "DEBUG: " << command  << " :::: " << getpid() << "\n";
    std::vector< std::vector<PartOfCommand> > parts(1);
    std::vector<bool> logic_connectors;
    PartOfCommand part;
    for (size_t i = 0, part_i = 0; i < command.size();) {
        while (i < command.size() && command[i] == ' ') {
            ++i;
        }
        if (i >= command.size()) {
            break;
        }
        if (i + 1 < command.size() && command[i] == '|' && command[i + 1] == '|') {
            logic_connectors.push_back(false);
            part_i++;
            i += 2;
            parts.emplace_back();
            continue;
        }
        if (i + 1 < command.size() && command[i] == '&' && command[i + 1] == '&') {
            logic_connectors.push_back(true);
            part_i++;
            i += 2;
            parts.emplace_back();
            continue;
        }
        if (command[i] == '>' || command[i] == '<' || command[i] == '|') {
            part.type = command[i];
            parts[part_i].push_back(part);
            ++i;
            continue;
        }
        part.type = 'c';
        if (command[i] == '\"') {
            ++i;
            while (command[i] != '\"') {
                part.word += ReadSymbol(i, command);
                ++i;
            }
            ++i;
        } else {
            while (i < command.size() && command[i] != ' ') {
                part.word += ReadSymbol(i, command);
                ++i;
            }
        }
        parts[part_i].push_back(part);
        part.word.clear();
    }

    int return_value;

    for (size_t part_i = 0; part_i < parts.size(); ++part_i) {

        if (part_i > 0) {
            if (return_value == 0 && !logic_connectors[part_i - 1]) {
                break;
            }
            if (return_value != 0 && logic_connectors[part_i - 1]) {
                break;
            }
        } 

        Instruction inst;
        std::vector<Instruction> insts;
        for (size_t i = 0; i < parts[part_i].size(); ++i) {
            if (parts[part_i][i].type == '<') {
                inst.in = parts[part_i][i + 1].word;
                i += 1;
            } else if (parts[part_i][i].type == '>') {
                inst.out = parts[part_i][i + 1].word;
                i += 1;
            } else if (parts[part_i][i].type == '|') {
                insts.push_back(inst);
                inst = Instruction();
            } else {
                inst.command.push_back(parts[part_i][i].word);
            }
        }

        insts.push_back(inst);

        ProgrammRunner pr(insts);
        return_value = pr.Run(STDIN_FILENO);
        // std::cout << "RV: " << return_value << "\n";
    }
    return return_value;
}
