#include "cgi.hpp"

CGI::CGI() {}

CGI::~CGI() {}

std::string CGI::cgiExecute(std::string location, std::string executable ,std::string	postname[], std::string	postvalue[], int j,std::string m_request, int code){
    code = 500;
    std::vector<std::string> env;
    std::string content = m_request;
    //new combine avec j elem;
    std::string combine[10000];
    //on part de lindice 0 svp
    for (int nm = 1; nm != j+1 ; nm++)
        combine[nm] = postname[nm]+ "=" + postvalue[nm];
    char cwd[256];
    for (int nm = 1; nm != j+1 ; nm++)
        combine[nm] = postname[nm]+ "=" + postvalue[nm];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        std::cerr << "Cgi can't get cwd" << std::endl;
		return ("Status: 500\r\n\r\n");
    }
    //variable useless
    env.push_back("CONTENT_TYPE text/html");
    env.push_back("DOCUMENT_ROOT");
    env.push_back("CONTENT_LENGTH");
    
    env.push_back("HTTP_COOKIE = none");
    env.push_back("HTTP_HOST = off");
    env.push_back("HTTP_REFERER");
    env.push_back("HTTP_USER_AGENT");
    env.push_back("HTTPS = off");
    env.push_back("PATH" + std::string(cwd));
    env.push_back("QUERY_STRING");
    
    env.push_back("REMOTE_ADDR");
    env.push_back("REMOTE_HOST");
    env.push_back("REMOTE_PORT");
    env.push_back("REMOTE_USER");
    env.push_back("REQUEST_METHOD");
    env.push_back("REQUEST_URI" + std::string(cwd));
    
    env.push_back("SCRIPT_FILENAME" ); 
    env.push_back("SCRIPT_NAME");
    
    env.push_back("SERVER_ADMIN");
    env.push_back("SERVER_NAME");
    env.push_back("SERVER_PORT");
    env.push_back("SERVER_SOFTWARE");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");

    for (int k = 1; k != j+1 ; k++)
        env.push_back(combine[k]);
    env.push_back("REDIRECT_STATeeUS=200");
    int pip[2];
    int ret = 0;
    pid_t child = 0;
    pid_t parent = 0;
    int fd = open("tmp", O_CREAT | O_TRUNC | O_WRONLY | O_NONBLOCK, 0777);
    if (fd < 0)
    {
        std::cerr << "Cgi open failed" << std::endl;
		return ("Status: 500\r\n\r\n");
    }

    int checke_Cwrite = write(fd, content.c_str(), content.size());
    if (checke_Cwrite < 0)
    {
        std::cerr << "Cgi can't write" << std::endl;
		return ("Status: 500\r\n\r\n");
    }

    if (close(fd) < 0)
    {
        std::cerr << "Cgi can't close" << std::endl;
		return ("Status: 500\r\n\r\n");
    }

    size_t i = 0;

    char **yes = new char *[env.size() + 1];

    for (; i != env.size(); i++)
        yes[i] = (char *)env.at(i).c_str();
    yes[i] = NULL;

    char *echo[3] = {(char *)"cat", (char *)"tmp", NULL};
    char *cmd[] =  {&location[0], &executable[0], NULL};
    if (pipe(pip) == -1)
    {
        std::cerr << "Cgi pipe failed" << std::endl;
		return ("Status: 500\r\n\r\n");
    }
    child = fork();
    int tmp = open(".tmp", O_CREAT | O_TRUNC | O_NONBLOCK | O_RDWR, 0777);
    if (tmp < 0)
    {
        std::cerr << "Cgi can't create tmp" << std::endl;
		return ("Status: 500\r\n\r\n");
    }
    if (child == -1)
    {
        std::cerr << "Fork failed" << std::endl;
		return ("Status: 500\r\n\r\n");
    }
    else if (!child)
    {
        if (dup2(pip[1], 1) < 0)
        {
            std::cerr << "Cgi can't dup" << std::endl;
		    return ("Status: 500\r\n\r\n");
        }
        if (close(pip[0]) < 0)
        {
            std::cerr << "Cgi can't close" << std::endl;
		    return ("Status: 500\r\n\r\n");;
        }
        ret = execvp(echo[0], echo);
        std::cerr << "Cgi can't exec" << std::endl;
		return ("Status: 500\r\n\r\n");
    }
    else
    {
        int status2;

        wait(&status2);
        parent = fork();
        if (parent == -1)
        {
            std::cerr << "Fork failed" << std::endl;
            return ("Status: 500\r\n\r\n");
        }
        if (!parent)
        {
            if (dup2(pip[0], 0) < 0)
            {
                std::cerr << "Cgi can't dup" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (dup2(tmp, 1) < 0)
            {
                std::cerr << "Cgi can't dup" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (close(pip[1]) < 0)
            {
                perror("CGI part : can't close file");
                exit(1);
            }
            ret = execve(cmd[0], cmd, yes);
            std::cerr << "Cgi can't exec" << std::endl;
		    return ("Status: 500\r\n\r\n");
        }
        else
        {
            int status;

            wait(&status);
            if (close(pip[0]) < 0)
            {
                std::cerr << "Cgi can't close" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (close(pip[1]) < 0)
            {
                std::cerr << "Cgi can't close" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (close(tmp) < 0)
            {
                std::cerr << "Cgi can't close" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }

            usleep(100000);

            tmp = open(".tmp", O_NONBLOCK | O_RDONLY);
            if (tmp < 0)
            {
                std::cerr << "Cgi can't create tmp" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }

            char buf[65535];
            bzero(buf, 65535);
            int checkRead = read(tmp, buf, 65535);
            if (checkRead < 0)
            {
                std::cerr << "Cgi can't read tmp" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (close(tmp) < 0)
            {
                std::cerr << "Cgi can't close" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            content = std::string(buf);
            if (remove("tmp") != 0)
            {
                std::cerr << "Cgi can't remove" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            if (remove(".tmp") != 0)
            {
                std::cerr << "Cgi can't remove" << std::endl;
		        return ("Status: 500\r\n\r\n");
            }
            delete[] yes;
            return (content);
        }
    }

    delete[] yes;
    return content;
}
