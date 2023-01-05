#include <command.h>
#include <filesystem>
#include <memory>
#include <utils.h>

#include <signal.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

namespace metabuild
{
    [[noreturn]] static void do_exec(const std::filesystem::path& name, const program_arguments& args, const program_environment& env)
    {
        // set up argv
        std::string a0 = name.string();
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(a0.c_str()));
        for (size_t i = 0; i < args.size(); i++)
            argv.push_back(const_cast<char*>(args[i].c_str()));
        argv.push_back(nullptr);

        // set up envp
        std::vector<char*> envp;

        // copy current environ
        auto curr_env = environ;
        while (*curr_env)
            envp.push_back(*curr_env++);

        // add the other stuff
        for (const auto& i : env)
        {
            std::string res = i.first + "=" + i.second;
            envp.push_back(strcpy(new char[res.size() + 1], res.c_str()));
        }

        envp.push_back(nullptr);

        execve(a0.c_str(), argv.data(), envp.data());
        exit(-1);
    }

    METABUILD_PUBLIC lazy<program_result> command::invoke_async(const program_arguments& args, const program_environment& env) const
    {
        pid_t child_pid = fork();

        if (child_pid == -1)
            throw std::system_error(errno, std::system_category());
        else if (child_pid == 0)
            do_exec(name, args, env);

        return [child_pid]() -> program_result {
            int status;
            while (waitpid(child_pid, &status, WUNTRACED) == -1)
            {
                if (errno != EINTR)
                    throw std::system_error(errno, std::system_category());
            }
            return status;
        };
    }

    METABUILD_PUBLIC program_result command::invoke(const program_arguments& args, const program_environment& env, std::string& out, std::string& err) const
    {
        int pipe_stdout[2];
        int pipe_stderr[2];

        if (pipe(pipe_stdout) < 0)
            throw std::system_error(errno, std::system_category());
        if (pipe(pipe_stderr) < 0)
            throw std::system_error(errno, std::system_category());

        pid_t child_pid = fork();

        if (child_pid == -1)
            throw std::system_error(errno, std::system_category());
        else if (child_pid == 0)
        {
            close(pipe_stdout[0]);
            close(pipe_stderr[0]);
            dup2(pipe_stdout[1], 1);
            dup2(pipe_stderr[1], 2);

            do_exec(name, args, env);
        }

        int status = 0;
        while (waitpid(child_pid, &status, WUNTRACED) == -1)
        {
            if (errno != EINTR)
                throw std::system_error(errno, std::system_category());
        }

        close(pipe_stdout[1]);
        close(pipe_stderr[1]);

        std::unique_ptr<char[]> buf(new char[100]);
        memset(buf.get(), 0, 100);

        int bytes_read = 0;

        while ((bytes_read = read(pipe_stdout[0], buf.get(), 99)))
        {
            if (bytes_read < 0 && errno != EINTR)
                throw std::system_error(errno, std::system_category());
            out += buf.get();
        }

        while ((bytes_read = read(pipe_stderr[0], buf.get(), 99)))
        {
            if (bytes_read < 0 && errno != EINTR)
                throw std::system_error(errno, std::system_category());
            err += buf.get();
        }

        return status;
    }
} // namespace metabuild
