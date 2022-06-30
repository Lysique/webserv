#ifndef CGI_HPP
# define CGI_HPP

#include "../config/ParserConfig.hpp"
#include "../request/ParserRequest.hpp"
#include "Utils.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>




class Server;

class CGI{
    public:
        CGI();
        ~CGI();

        std::string cgiExecute(std::string location, std::string executable ,std::string	postname[], std::string	postvalue[], int j,std::string m_request, int code);
};

#endif
